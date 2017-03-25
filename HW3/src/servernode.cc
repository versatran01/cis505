#include "servernode.hpp"
#include "chatutils.hpp"
#include "loguru.hpp"
#include "lpi.h"
#include "string_algorithms.hpp"
#include <experimental/filesystem>
#include <fstream>
namespace fs = std::experimental::filesystem;

static constexpr int kMaxBufSize = 1024;

std::vector<Server> ParseConfig(const std::string &config) {
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "cwd, path={%s}", cwd.c_str());
  fs::path config_file = cwd / config;
  if (!fs::exists(config_file)) {
    throw std::invalid_argument("Config doesn't exist.");
  }

  std::vector<Server> servers;
  std::fstream infile(config_file);
  std::string line;
  while (std::getline(infile, line)) {
    // Skip empty line
    trim(line);
    if (line.empty()) {
      LOG_F(WARNING, "Empty line in config={%s}", config.c_str());
      continue;
    }

    // Try to split each line input forward and binding address and port
    std::string forward_addr, binding_addr;
    std::tie(forward_addr, binding_addr) = GetServerAddress(line);

    // Parse Addr and port and add to server list
    try {
      const auto forward = ParseAddress(forward_addr);
      const auto binding = ParseAddress(binding_addr);
      servers.emplace_back(forward, binding);
      LOG_F(INFO, "forward={%s}, binding={%s}", forward.full_str().c_str(),
            binding.full_str().c_str());
    } catch (const std::invalid_argument &err) {
      LOG_F(ERROR, err.what());
      continue;
    }
  }

  return servers;
}

ServerNode::ServerNode(int id, const std::string &order) : id_(id) {
  if (order == "unordered") {
    order_ = Order::UNORDERD;
    LOG_F(INFO, "unordered");
  } else if (order == "fifo") {
    order_ = Order::FIFO;
    LOG_F(INFO, "fifo order");
  } else if (order == "total") {
    order_ = Order::TOTAL;
    LOG_F(INFO, "fifo order");
  } else {
    order_ = Order::UNORDERD;
    LOG_F(WARNING, "invalid order, default to unordered");
  }
}

void ServerNode::Init(const std::string &config) {
  ReadConfig(config);
  SetupConnection();
}

void ServerNode::SetupConnection() {
  // Setup connection
  const Address &binding = servers_[index()].binding();
  fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_ == -1) {
    LOG_F(ERROR, "[S%d] Failed to create socket", id());
    errExit("Failed to create socket.");
  }

  // Bind to binding address
  auto serv_addr = MakeSockAddrInet(binding);
  if (bind(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    LOG_F(ERROR, "[S%d] Failed to bind to address", id());
    errExit("Failed to bind to address");
  }

  LOG_F(INFO, "[S%d] Server addr={%s}", id(), binding.full_str().c_str());
}

void ServerNode::ReadConfig(const std::string &config) {
  try {
    servers_ = ParseConfig(config);
  } catch (const std::invalid_argument &err) {
    LOG_F(ERROR, err.what());
    errExit(err.what());
  }
  LOG_F(INFO, "Total servers, n={%zu}", n_servers());
}

void ServerNode::Run() {
  while (true) {
    Address addr;
    std::string msg;
    if (!RecvFrom(addr, msg)) {
      continue;
    }

    if (IsFromServer(addr)) {
      LOG_F(INFO, "[S%d] Msg from server, addr={%s}", id(),
            addr.full_str().c_str());
      HandleServerMsg(addr, msg);
    } else {
      LOG_F(INFO, "[S%d] Msg from client, addr={%s}", id(),
            addr.full_str().c_str());
      HandleClientMsg(addr, msg);
    }
  }
}

int ServerNode::GetServerIndex(const Address &addr) const {
  auto cmp_addr = [&](const Server &srv) { return srv.binding() == addr; };
  auto it = std::find_if(servers_.begin(), servers_.end(), cmp_addr);
  if (it == servers_.end()) {
    return -1;
  } else {
    // TODO: need to verify this
    return std::distance(servers_.begin(), it);
  }
}

int ServerNode::GetClientIndex(const Address &addr) const {
  auto cmp_addr = [&](const Client &client) { return client.addr() == addr; };
  auto it = std::find_if(clients_.begin(), clients_.end(), cmp_addr);
  if (it == clients_.end()) {
    return -1;
  } else {
    // TODO: need to verify this
    return std::distance(clients_.begin(), it);
  }
}

void ServerNode::HandleClientMsg(const Address &addr, const std::string &msg) {
  if (msg.empty()) {
    LOG_F(WARNING, "[S%d] empty string", id());
    return;
  }

  // Try to get index
  auto client_index = GetClientIndex(addr);
  if (client_index < 0) {
    clients_.emplace_back(addr);
    client_index = clients_.size() - 1;
    LOG_F(INFO, "[S%d] Add a new client, addr={%s}", id(),
          addr.full_str().c_str());
  }
  Client &client = clients_[client_index];

  // Check if the first character is a /
  if (msg[0] == '/') {
    // This is a command
    const auto cmd = trim_copy(msg.substr(1, 5));
    if (cmd == "join") {
      const auto room = msg.substr(6);
      Join(client, room);
    } else if (cmd == "nick") {
      const auto nick = msg.substr(6);
      Nick(client, nick);
    } else if (cmd == "part") {
      Part(client);
    } else if (cmd == "quit") {
      Quit(client);
    } else {
      LOG_F(WARNING, "[S%d] invalid command cmd={%s}", id(), cmd.c_str());
      ReplyErr(addr, "You entered an invalid command.");
    }
    LOG_F(INFO, "[S%d] Num clients, n={%zu}", id(), n_clients());
  } else {
    // Forward message to all servers
    ForwardMsgToServers(msg);
  }
}

void ServerNode::SendMsgToClient(const Client &client,
                                 const std::string &msg) const {
  SendTo(client.addr(), client.nick_prefix() + msg);
}

void ServerNode::SendMsgToAllClients(const std::string &msg) const {
  for (const Client &client : clients_) {
    SendMsgToClient(client, msg);
  }
}

void ServerNode::ForwardMsgToServers(const std::string &msg) const {
  for (const Server &server : servers_) {
    SendTo(server.forward(), msg);
    LOG_F(INFO, "[S%d] Send to forward, addr={%s}", id(),
          server.forward().full_str());
  }
}

bool ServerNode::SendTo(const Address &addr, const std::string &msg) const {
  auto dest = MakeSockAddrInet(addr);
  auto nsend = sendto(fd_, msg.c_str(), msg.size(), 0, (struct sockaddr *)&dest,
                      sizeof(dest));
  if (nsend < 0) {
    LOG_F(ERROR, "[S%d] Send failed, dest={%s}", id(), addr.full_str().c_str());
    return false;
  } else if (nsend != static_cast<int>(msg.size())) {
    LOG_F(WARNING,
          "[S%d] Byte sent doesn't match msg size, nsend={%zu}, nmsg={%zu}",
          id_, nsend, msg.size());
    return false;
  } else {
    LOG_F(INFO, "[S%d] Msg sent, str={%s}, addr={%s}", id(), msg.c_str(),
          addr.full_str().c_str());
    return true;
  }
}

bool ServerNode::RecvFrom(Address &addr, std::string &msg) const {
  struct sockaddr_in src;
  socklen_t srclen = sizeof(src);
  char buffer[kMaxBufSize];
  int nrecv = recvfrom(fd_, buffer, sizeof(buffer) - 1, 0,
                       (struct sockaddr *)&src, &srclen);
  if (nrecv < 0) {
    LOG_F(ERROR, "[S%d] recvfrom failed", id());
    return false;
  }

  // Null terminate buffer
  buffer[nrecv] = 0;
  msg = std::string(buffer, nrecv);
  addr = MakeAddress(src);
  LOG_F(INFO, "[S%d] Read, str={%s}, n={%d}, addr={%s}", id(), msg.c_str(),
        nrecv, addr.full_str().c_str());
  return true;
}

void ServerNode::ReplyOk(const Address &addr, const std::string &msg) const {
  SendTo(addr, "+OK " + msg);
}

void ServerNode::ReplyErr(const Address &addr, const std::string &msg) const {
  SendTo(addr, "-ERR " + msg);
}

void ServerNode::ReplyOk(const Client &client, const std::string &msg) const {
  ReplyOk(client.addr(), msg);
}

void ServerNode::ReplyErr(const Client &client, const std::string &msg) const {
  ReplyErr(client.addr(), msg);
}

void ServerNode::Join(Client &client, const std::string &arg) {
  const int room = std::atoi(arg.c_str());
  if (room <= 0) {
    LOG_F(ERROR, "[S%d] Invalid room number, arg={%s}", id(), arg.c_str());
    ReplyErr(client, "You provided an invalid room number #" + arg);
    return;
  }

  if (client.InRoom()) {
    LOG_F(WARNING, "[S%d] client already in room, nick={%s}, room={%d}", id(),
          client.nick().c_str(), client.room());
    // If already in room reply with error message
    ReplyErr(client, "You are already in chat room #" + client.room_str());
  } else {
    // If not, join room
    client.Join(room);
    LOG_F(INFO, "[S%d] client join room, nick={%s}, room={%d}", id(),
          client.nick().c_str(), client.room());
    ReplyOk(client, "You are now in chat room #" + arg);
  }
}

void ServerNode::Nick(Client &client, const std::string &arg) {
  if (arg.empty()) {
    LOG_F(ERROR, "[S%d] No nick name provided", id());
    ReplyErr(client, "You provided an empty nick name");
    return;
  }

  client.set_nick(arg);
  LOG_F(INFO, "[S%d] Client changed nick, nick={%s}", id(),
        client.nick().c_str());
  ReplyOk(client, "Nick name set to '" + arg + "'");
}

void ServerNode::Part(Client &client) {
  const auto old_room = client.Leave();
  if (old_room < 0) {
    LOG_F(WARNING, "[S%d] Client is not in a room, nick={%s}", id(),
          client.nick().c_str());
    ReplyErr(client, "You are not in any room");
  } else {
    LOG_F(INFO, "[S%d] Client left room, nick={%s}, room={%d}", id(),
          client.nick().c_str(), old_room);
    ReplyOk(client, "You have left chat room #" + std::to_string(old_room));
  }
}

void ServerNode::Quit(Client &client) {
  LOG_F(INFO, "[S%d] Removing client, addr={%s}", id(),
        client.addr_str().c_str());
  // Remove client
  clients_.erase(std::remove(clients_.begin(), clients_.end(), client),
                 clients_.end());
}

void ServerNode::HandleServerMsg(const Address &addr, const std::string &msg) {
  // Simply send messages to all clients with nick name
  for (const Client &client : clients_) {
    SendMsgToClient(client, msg);
  }
}

bool ServerNode::IsFromServer(const Address &addr) const {
  auto cmp_addr = [&](const Server &srv) { return srv.forward() == addr; };
  auto it = std::find_if(servers_.begin(), servers_.end(), cmp_addr);
  return it != servers_.end();
}
