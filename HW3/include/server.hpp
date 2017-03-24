#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

#include "addrport.hpp"
#include "client.hpp"

enum class Order { UNORDERD, FIFO, TOTAL };

class Server {
public:
  Server(int id, const std::string &order);

  void Init(const std::string &config);
  void Run();

  int fd() const { return fd_; }
  int id() const { return id_; }
  int index() const { return id_ - 1; }

  const std::vector<ServerAddrPort> &servers() const { return servers_; }
  const std::vector<Client> &clients() const { return clients_; }

  size_t n_servers() const { return servers_.size(); }
  size_t n_clients() const { return clients_.size(); }

private:
  void ReadConfig(const std::string &config);
  void SetupConnection();

  int GetServerIndex(const AddrPort &addrport) const;
  int GetClientIndex(const AddrPort &addrport) const;

  void SendTo(const AddrPort &addrport, const std::string &msg) const;
  void RecvFrom(AddrPort &addrport, std::string &msg) const;

  void ReplyOk(const AddrPort &addrport, const std::string &msg) const;
  void ReplyErr(const AddrPort &addrport, const std::string &msg) const;

  void HandleClientMessage(const AddrPort &src, const std::string &msg);
  void ForwardMessage(const std::string &msg) const;

  void Join();
  void Nick();
  void Part();
  void Quit();

  int fd_;
  int id_; // id of the server, starts at 1
  std::vector<ServerAddrPort> servers_;
  std::vector<Client> clients_;
  Order order_;
};

#endif // SERVER_HPP
