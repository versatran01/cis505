#include "server.hpp"
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
    LOG_F(INFO, "forward={%s}, binding={%s}", forward_addr_port.c_str(),
          binding_addr_port.c_str());

    // Parse Addr and port and add to server list
    try {
      const auto forward = ParseAddrPort(forward_addr_port);
      const auto binding = ParseAddrPort(binding_addr_port);
      server_list.emplace_back(forward, binding);
      LOG_F(INFO, "Add forward addr={%s}, port={%d}", forward.addr().c_str(),
            forward.port());
      LOG_F(INFO, "Add binding addr={%s}, port={%d}", binding.addr().c_str(),
            forward.port());
    } catch (const std::invalid_argument &err) {
      LOG_F(ERROR, err.what());
      continue;
    }
  }

  return server_list;
}

Server::Server(int index) : index_(index) {}

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
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);
    char buffer[kMaxBufSize];
    int nrecv = recvfrom(fd_, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&src, &srclen);
    if (nrecv < 0) {
      LOG_F(ERROR, "[S%d] recvfrom failed", index_);
      continue;
    }

    // Null terminate buffer
    buffer[nrecv] = 0;
    std::string msg(buffer, nrecv);
    LOG_F(INFO, "[S%d] Read, str={%s}, n={%d}", index_, msg.c_str(), nrecv);
    const auto src_addr_port = GetAddrPort(src);

    //    const auto src_index = GetServerIndex(src_addr_port);
    sendto(fd_, buffer, nrecv, 0, (struct sockaddr *)&src, sizeof(src));
  }
}
