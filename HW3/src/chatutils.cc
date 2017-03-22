#include "chatutils.hpp"

#include <experimental/filesystem>
#include <fstream>
namespace fs = std::experimental::filesystem;

#include "loguru.hpp"
#include "string_algorithms.hpp"

AddrPort ParseAddrPort(const std::string &addr_port) {
  static const char *pattern =
      "^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]+)\\:([0-9]{1,5})$";
  std::regex addr_port_regex(pattern);

  std::smatch results;
  if (!std::regex_search(addr_port, results, addr_port_regex)) {
    throw std::invalid_argument("Ivalid address and port");
  }

  // Port must be a number because of the regex
  return {results.str(1), std::atoi(results.str(2).c_str())};
}

std::tuple<std::string, std::string>
GetForwardBinding(const std::string &line) {
  const auto delim_index = line.find(',');
  if (delim_index == std::string::npos) {
    // Same address
    return std::make_tuple(line, line);
  } else {
    // Different addresses
    return std::make_tuple(trim_copy(line.substr(0, delim_index)),
                           trim_copy(line.substr(delim_index + 1)));
  }
}

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
