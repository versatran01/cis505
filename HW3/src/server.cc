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
      LOG_F(INFO, "forward={%s}, binding={%s}", forward.addr_port().c_str(),
            binding.addr_port().c_str());
    } catch (const std::invalid_argument &err) {
      LOG_F(ERROR, err.what());
      continue;
    }
  }

  return server_list;
}

Server::Server(int index, const std::string &order) : index_(index) {
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
  const auto &binding = servers_[index_ - 1].binding();
  fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_ == -1) {
    LOG_F(ERROR, "[S%d] Failed to create socket", index_);
    errExit("Failed to create socket.");
  }

  // Bind to binding address
  auto serv_addr = MakeSockAddrInet(binding);
  if (bind(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    LOG_F(ERROR, "[S%d] Failed to bind to address", index_);
    errExit("Failed to bind to address");
  }

  LOG_F(INFO, "[S%d] Server addr={%s}", index_, binding.addr_port().c_str());
}

void Server::ReadConfig(const std::string &config) {
  try {
    servers_ = ParseConfig(config);
  } catch (const std::invalid_argument &err) {
    LOG_F(ERROR, err.what());
    errExit(err.what());
  }
  LOG_F(INFO, "Total servers, n={%zu}", servers_.size());
}

void Server::Run() {
  while (true) {
    struct sockaddr_in src_addr;
    socklen_t srclen = sizeof(src_addr);
    char buffer[kMaxBufSize];
    int nrecv = recvfrom(fd_, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&src_addr, &srclen);
    if (nrecv < 0) {
      LOG_F(ERROR, "[S%d] recvfrom failed", index_);
      continue;
    }

    // Null terminate buffer
    buffer[nrecv] = 0;
    std::string msg(buffer, nrecv);
    const auto src_addr_port = GetAddrPort(src_addr);
    LOG_F(INFO, "[S%d] Read, str={%s}, n={%d}, addr={%s}", index_, msg.c_str(),
          nrecv, src_addr_port.addr_port().c_str());

    // Check src address
    auto src_index = GetServerIndex(src_addr_port);
    if (src_index < 0) {
      LOG_F(INFO, "[S%d] Msg from client, addr={%s}", index_,
            src_addr_port.addr_port().c_str());
      HandleClientMessage(src_addr_port, msg);
    } else {
      // TODO message from server
    }

    sendto(fd_, buffer, nrecv, 0, (struct sockaddr *)&src_addr,
           sizeof(src_addr));
  }
}

int Server::GetServerIndex(const AddrPort &addr_port) const {
  auto cmp_addr_port = [&](const ServerAddrPort &srv) {
    return srv.binding() == addr_port;
  };
  auto it = std::find_if(servers_.begin(), servers_.end(), cmp_addr_port);
  if (it == servers_.end()) {
    return -1;
  } else {
    // TODO: need to verify this
    return std::distance(servers_.begin(), it);
  }
}

void Server::HandleClientMessage(const AddrPort &src, const std::string &msg) {
  if (msg.empty()) {
    LOG_F(WARNING, "[S%d] empty string", index_);
    return;
  }

  // Check if the first character is a /
  if (msg[0] == '/') {
    // This is a command
    const auto cmd = msg.substr(1, 4);
    const auto arg = msg.substr(6);
    if (cmd == "join") {
      const int room_index = std::atoi(arg.c_str());
      if (room_index <= 0) {
        LOG_F(ERROR, "[S%d] Invalid room number, arg={%s}", index_,
              arg.c_str());
        return;
      }

      // Check if this client is in some chatroom already

    } else if (cmd == "nick") {
    } else if (cmd == "part") {
    } else if (cmd == "quit") {

    } else {
      LOG_F(WARNING, "[S%d] invalid command cmd={%s}", index_, cmd.c_str());
    }
  } else {
    // This is a message, just forward to every other server
    ForwardMessage(msg);
    // Also send it back to itself
  }
}

void Server::ForwardMessage(const std::string &msg) const {
  int index = index_ - 1;
  for (size_t i = 0; i < servers_.size(); ++i) {
    const auto server = servers_[i];
    if (i != index) {
      SendTo(server.forward(), msg);
      LOG_F(INFO, "[S%d] Send to i={%d}", index_, i + 1);
    }
  }
}

void Server::SendTo(const AddrPort &addr_port, const std::string &msg) const {
  auto dest = MakeSockAddrInet(addr_port);
  auto nsend = sendto(fd_, msg.c_str(), msg.size(), 0, (struct sockaddr *)&dest,
                      sizeof(dest));
  if (nsend < 0) {
    LOG_F(ERROR, "[S%d] Send failed, dest={%s}", index_,
          addr_port.addr_port().c_str());
  } else if (nsend != msg.size()) {
    LOG_F(WARNING,
          "[S%d] Byte sent doesn't match msg size, nsend={%zu}, nmsg={%zu}",
          index_, nsend, msg.size());
  } else {
    LOG_F(INFO, "[S%d] Msg sent, str={%s}, addr={%s}", index_, msg.c_str(),
          addr_port.addr_port().c_str());
  }
}
