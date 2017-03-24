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

  int GetServerIndex(const AddrPort &addr) const;
  int GetClientIndex(const AddrPort &addr) const;

  void SendTo(const AddrPort &addr, const std::string &msg) const;
  void RecvFrom(AddrPort &addr, std::string &msg) const;

  void ReplyOk(const AddrPort &addr, const std::string &msg) const;
  void ReplyErr(const AddrPort &addr, const std::string &msg) const;

  void SendMsgToClient(const Client &client, const std::string &msg) const;
  void SendMsgToAllClients(const std::string &msg) const;

  void HandleClientMessage(const AddrPort &addr, const std::string &msg);
  void HandleServerMessage(const AddrPort &addr, const std::string &msg);
  void ForwardMsgToServers(const std::string &msg) const;

  void Join(const AddrPort &addr, const std::string &arg);
  void Nick(const AddrPort &addr, const std::string &arg);
  void Part(const AddrPort &addr);
  void Quit(const AddrPort &addr);

  int fd_;
  int id_; // id of the server, starts at 1
  std::vector<ServerAddrPort> servers_;
  std::vector<Client> clients_;
  Order order_;
};

#endif // SERVER_HPP
