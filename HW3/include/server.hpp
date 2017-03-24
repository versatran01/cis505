#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

#include "addrport.hpp"

enum class Order { UNORDERD, FIFO, TOTAL };

class Server {
public:
  Server(int index, const std::string &order);

  void Init(const std::string &config);
  void Run();

  int index() const { return index_; }

private:
  void ReadConfig(const std::string &config);
  void SetupConnection();
  int GetServerIndex(const AddrPort &addr_port) const;
  void HandleClientMessage(const AddrPort &src, const std::string &msg);
  void SendTo(const AddrPort &addr_port, const std::string &msg) const;
  void RecvFrom(AddrPort &addr_port, std::string &msg) const;
  void ForwardMessage(const std::string &msg) const;

  int fd_;
  int index_;
  std::vector<ServerAddrPort> servers_;
  std::vector<AddrPort> clients_;
  Order order_;
};

#endif // SERVER_HPP
