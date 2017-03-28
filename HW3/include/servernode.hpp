#ifndef SERVERNODE_HPP
#define SERVERNODE_HPP

#include <string>
#include <vector>

#include "client.hpp"
#include "holdbackqueue.hpp"
#include "message.hpp"
#include "server.hpp"
#include "totalorder.hpp"

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
   * @brief Run server node, start from here
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
  void HandleClientMsg(const Address &addr, const std::string &msg);
  void HandleServerMsg(const Address &addr, const std::string &msg);

  void SendMsgToClient(const Client &client, const std::string &msg) const;
  void Deliver(int room, const std::string &msg) const;
  void Multicast(const std::string &msg) const;

  void ReadConfig(const std::string &config);
  void SetupConnection();

  int GetServerIndex(const Address &addr) const;
  int GetClientIndex(const Address &addr) const;
  bool IsFromServer(const Address &addr) const;

  /// Simplify communication
  bool SendTo(const Address &addr, const std::string &msg) const;
  bool RecvFrom(Address &addr, std::string &msg) const;

  void ReplyOk(const Address &addr, const std::string &msg) const;
  void ReplyErr(const Address &addr, const std::string &msg) const;
  void ReplyOk(const Client &client, const std::string &msg) const;
  void ReplyErr(const Client &client, const std::string &msg) const;

  /// Handle client commands
  void Join(Client &client, const std::string &arg);
  void Nick(Client &client, const std::string &arg);
  void Part(Client &client);
  void Quit(Client &client);

  /// Handle different ordering
  void UnorderedDeliver(const std::string &msg);
  void FifoDeliver(const std::string &msg);
  void TotalOrderDeliver(const std::string &msg);

  int fd_;
  int id_; // id of the server, starts at 1
  std::vector<Server> servers_;
  std::vector<Client> clients_;
  Order order_;
  // FIFO related
  std::map<std::string, std::map<int, int>> seq_fifo_;
  HoldbackQueueFIFO hbq_fifo_;
  // Total order
  TotalOrder totalorder_;
};

#endif // SERVERNODE_HPP
