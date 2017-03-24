#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>

#include "address.hpp"
#include "client.hpp"

enum class Order { UNORDERD, FIFO, TOTAL };

class ServerNode {
public:
  ServerNode(int id, const std::string &order);

  /**
   * @brief Init Read config and setup connection
   * @param config Config file
   */
  void Init(const std::string &config);

  /**
   * @brief Run server node
   */
  void Run();

  int fd() const { return fd_; }
  int id() const { return id_; }
  int index() const { return id_ - 1; }

  const std::vector<Server> &servers() const { return servers_; }
  const std::vector<Client> &clients() const { return clients_; }

  size_t n_servers() const { return servers_.size(); }
  size_t n_clients() const { return clients_.size(); }

private:
  void ReadConfig(const std::string &config);
  void SetupConnection();

  int GetServerIndex(const Address &addr) const;
  int GetClientIndex(const Address &addr) const;

  void SendTo(const Address &addr, const std::string &msg) const;
  void RecvFrom(Address &addr, std::string &msg) const;

  void ReplyOk(const Address &addr, const std::string &msg) const;
  void ReplyErr(const Address &addr, const std::string &msg) const;

  void SendMsgToClient(const Client &client, const std::string &msg) const;
  void SendMsgToAllClients(const std::string &msg) const;

  void HandleClientMessage(const Address &addr, const std::string &msg);
  void HandleServerMessage(const Address &addr, const std::string &msg);
  void ForwardMsgToServers(const std::string &msg) const;

  void Join(const Address &addr, const std::string &arg);
  void Nick(const Address &addr, const std::string &arg);
  void Part(const Address &addr);
  void Quit(const Address &addr);

  /**
   * @brief IsFromServer Check whether addr is forward addr of server
   * @param addr address of incoming message
   * @return True if message is from another server
   */
  bool IsFromServer(const Address &addr) const;

  int fd_;
  int id_; // id of the server, starts at 1
  std::vector<Server> servers_;
  std::vector<Client> clients_;
  Order order_;
};

#endif // SERVER_HPP
