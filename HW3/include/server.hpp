#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

#include "chatutils.hpp"

class Server {
public:
  explicit Server(int index);

  void Init(const std::string &config);
  void Run();

  int index() const { return index_; }

private:
  void ReadConfig(const std::string &config);
  void SetupConnection();

  int fd_;
  int index_;
  std::vector<ServerAddrPort> servers_;
};

#endif // SERVER_HPP
