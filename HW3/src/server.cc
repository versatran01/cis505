#include "server.hpp"
#include "chatutils.hpp"
#include "loguru.hpp"
#include "lpi.h"
#include "string_algorithms.hpp"
#include <experimental/filesystem>
#include <fstream>
namespace fs = std::experimental::filesystem;

static constexpr int kMaxBufSize = 1024;

std::vector<ServerAddrPort> ParseConfig(const std::string &config) {
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "cwd, path={%s}", cwd.c_str());
  fs::path config_file = cwd / config;
  if (!fs::exists(config_file)) {
    throw std::invalid_argument("Config doesn't exist.");
  }

  std::vector<ServerAddrPort> server_list;
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
    std::string forward_addr_port, binding_addr_port;
    std::tie(forward_addr_port, binding_addr_port) = GetForwardBinding(line);
    //    LOG_F(INFO, "forward={%s}, binding={%s}", forward_addr_port.c_str(),
    //          binding_addr_port.c_str());

    // Parse Addr and port and add to server list
    try {
      const auto forward = ParseAddrPort(forward_addr_port);
      const auto binding = ParseAddrPort(binding_addr_port);
      server_list.emplace_back(forward, binding);
      LOG_F(INFO, "forward={%s}, binding={%s}", forward.full().c_str(),
            binding.full().c_str());
    } catch (const std::invalid_argument &err) {
      LOG_F(ERROR, err.what());
      continue;
    }
  }

  return server_list;
}

Server::Server(int id, const std::string &order) : id_(id) {
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

void Server::Init(const std::string &config) {
  ReadConfig(config);
  SetupConnection();
}

void Server::SetupConnection() {
  // Setup connection
  const auto &binding = servers_[index()].binding();
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

  LOG_F(INFO, "[S%d] Server addr={%s}", id(), binding.full().c_str());
}

void Server::ReadConfig(const std::string &config) {
  try {
    servers_ = ParseConfig(config);
  } catch (const std::invalid_argument &err) {
    LOG_F(ERROR, err.what());
    errExit(err.what());
  }
  LOG_F(INFO, "Total servers, n={%zu}", n_servers());
}

void Server::Run() {
  while (true) {
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);
    char buffer[kMaxBufSize];
    int nrecv = recvfrom(fd_, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&src, &srclen);
    if (nrecv < 0) {
      LOG_F(ERROR, "[S%d] recvfrom failed", id());
      continue;
    }

    // Null terminate buffer
    buffer[nrecv] = 0;
    std::string msg(buffer, nrecv);
    const auto src_addr = GetAddrPort(src);
    LOG_F(INFO, "[S%d] Read, str={%s}, n={%d}, addr={%s}", id(), msg.c_str(),
          nrecv, src_addr.full().c_str());

    // Check src address
    const auto src_index = GetServerIndex(src_addr);
    if (src_index < 0) {
      LOG_F(INFO, "[S%d] Msg from client, addr={%s}", id(),
            src_addr.full().c_str());
      HandleClientMessage(src_addr, msg);
    } else {
      LOG_F(INFO, "[S%d] Msg from server, addr={%s}", id(),
            src_addr.full().c_str());
      // TODO message from server
      HandleServerMessage(src_addr, msg);
    }
  }
}

int Server::GetServerIndex(const AddrPort &addr) const {
  auto cmp_addr = [&](const ServerAddrPort &srv) {
    return srv.binding() == addr;
  };
  auto it = std::find_if(servers_.begin(), servers_.end(), cmp_addr);
  if (it == servers_.end()) {
    return -1;
  } else {
    // TODO: need to verify this
    return std::distance(servers_.begin(), it);
  }
}

int Server::GetClientIndex(const AddrPort &addr) const {
  auto cmp_addr = [&](const Client &client) { return client.addr() == addr; };
  auto it = std::find_if(clients_.begin(), clients_.end(), cmp_addr);
  if (it == clients_.end()) {
    return -1;
  } else {
    // TODO: need to verify this
    return std::distance(clients_.begin(), it);
  }
}

void Server::HandleClientMessage(const AddrPort &addr, const std::string &msg) {
  if (msg.empty()) {
    LOG_F(WARNING, "[S%d] empty string", id());
    return;
  }

  // Check if the first character is a /
  if (msg[0] == '/') {
    // This is a command
    const auto cmd = trim_copy(msg.substr(1, 5));
    if (cmd == "join") {
      const auto room = msg.substr(6);
      Join(addr, room);
    } else if (cmd == "nick") {
      const auto nick = msg.substr(6);
      Nick(addr, nick);
    } else if (cmd == "part") {
      Part(addr);
    } else if (cmd == "quit") {
      Quit(addr);
    } else {
      LOG_F(WARNING, "[S%d] invalid command cmd={%s}", id(), cmd.c_str());
      ReplyErr(addr, "You entered an invalid command.");
    }
    LOG_F(INFO, "[S%d] Num clients, n={%zu}", id(), n_clients());
  } else {
    // This is a message
    const auto client_index = GetClientIndex(addr);
    const Client &client = clients_[client_index];
    if (client_index >= 0) {
      // This message is from a client
      // Send it to all servers
      ForwardMsgToServers(msg);
      // And also send back to this client
      SendMsgToClient(client, msg);
    }
  }
}

void Server::SendMsgToClient(const Client &client,
                             const std::string &msg) const {
  SendTo(client.addr(), client.nick2() + msg);
}

void Server::SendMsgToAllClients(const std::string &msg) const {
  for (const Client &client : clients_) {
    SendMsgToClient(client, msg);
  }
}

void Server::ForwardMsgToServers(const std::string &msg) const {
  for (size_t i = 0; i < servers_.size(); ++i) {
    const auto server = servers_[i];
    const auto this_server = index();
    if (i != this_server) {
      SendTo(server.forward(), msg);
      LOG_F(INFO, "[S%d] Send to i={%d}", id(), i + 1);
    }
  }
}

void Server::SendTo(const AddrPort &addr, const std::string &msg) const {
  auto dest = MakeSockAddrInet(addr);
  auto nsend = sendto(fd_, msg.c_str(), msg.size(), 0, (struct sockaddr *)&dest,
                      sizeof(dest));
  if (nsend < 0) {
    LOG_F(ERROR, "[S%d] Send failed, dest={%s}", id(), addr.full().c_str());
  } else if (nsend != msg.size()) {
    LOG_F(WARNING,
          "[S%d] Byte sent doesn't match msg size, nsend={%zu}, nmsg={%zu}",
          id_, nsend, msg.size());
  } else {
    LOG_F(INFO, "[S%d] Msg sent, str={%s}, addr={%s}", id(), msg.c_str(),
          addr.full().c_str());
  }
}

void Server::ReplyOk(const AddrPort &addr, const std::string &msg) const {
  SendTo(addr, "+OK " + msg);
}

void Server::ReplyErr(const AddrPort &addr, const std::string &msg) const {
  SendTo(addr, "-ERR " + msg);
}

void Server::Join(const AddrPort &addr, const std::string &arg) {
  const int room = std::atoi(arg.c_str());
  if (room <= 0) {
    LOG_F(ERROR, "[S%d] Invalid room number, arg={%s}", id(), arg.c_str());
    ReplyErr(addr, "You provided an invalid room number #" + arg);
    return;
  }

  // Check if this client exists in the client list
  const auto client_index = GetClientIndex(addr);
  if (client_index < 0) {
    // If not, create a new client
    clients_.emplace_back(addr, room);
    LOG_F(INFO, "[S%d] Add a new client, addr={%s}, room={%d}", id(),
          addr.full().c_str(), room);
    ReplyOk(addr, "You are now in chat room #" + arg);
  } else {
    // If exists, check if it is already in a room
    Client &client = clients_[client_index];
    if (client.InRoom()) {
      LOG_F(WARNING, "[S%d] client already in room, nick={%s}, room={%d}", id(),
            client.nick().c_str(), client.room());
      // If already in room reply with error message
      ReplyErr(addr, "You are already in chat room #" + client.room_str());
    } else {
      // If not, join room
      client.Join(room);
      LOG_F(INFO, "[S%d] client join room, nick={%s}, room={%d}", id(),
            client.nick().c_str(), client.room());
      ReplyOk(addr, "You are now in chat room #" + arg);
    }
  }
}

void Server::Nick(const AddrPort &addr, const std::string &arg) {
  if (arg.empty()) {
    LOG_F(ERROR, "[S%d] No nick name provided", id());
    ReplyErr(addr, "You provided an empty nick name");
    return;
  }

  // Check if this client exists in the client list
  const auto client_index = GetClientIndex(addr);
  if (client_index < 0) {
    // If not, create a new client
    clients_.emplace_back(addr, arg);
    LOG_F(INFO, "[S%d] Add a new client, addr={%s}, nick={%s}", id(),
          addr.full().c_str(), arg.c_str());
  } else {
    Client &client = clients_[client_index];
    client.set_nick(arg);
    LOG_F(INFO, "[S%d] Client change nick, new={%s}", id(),
          client.nick().c_str());
  }
  ReplyOk(addr, "Nick name set to '" + arg + "'");
}

void Server::Part(const AddrPort &addr) {
  // Check if this client exists in the client list
  const auto client_index = GetClientIndex(addr);
  if (client_index < 0) {
    LOG_F(WARNING, "[S%d] Client hasn't joined any room", id());
    ReplyErr(addr, "Cannot part because you haven't join any room");
  } else {
    Client &client = clients_[client_index];
    const auto old_room = client.Leave();
    if (old_room < 0) {
      LOG_F(WARNING, "[S%d] Client is not in a room, nick={%s}", id(),
            client.nick().c_str());
      ReplyErr(addr, "You are not in any room");
    } else {
      LOG_F(INFO, "[S%d] Client left room, nick={%s}, room={%d}", id(),
            client.nick().c_str(), old_room);
      ReplyOk(addr, "You have left chat room #" + std::to_string(old_room));
    }
  }
}

void Server::Quit(const AddrPort &addr) {
  const auto client_index = GetClientIndex(addr);
  if (client_index < 0) {
    LOG_F(WARNING, "[S%d] Client doesn't exists", id());
    return;
  } else {
    LOG_F(INFO, "[S%d] Removing client index={%d}", id(), client_index);
    // Remove client
    auto cmp_addr = [&addr](const Client &client) {
      return client.addr() == addr;
    };
    clients_.erase(std::remove_if(clients_.begin(), clients_.end(), cmp_addr),
                   clients_.end());
  }
}

void Server::HandleServerMessage(const AddrPort &addr, const std::string &msg) {
}
