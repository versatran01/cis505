#include "servernode.hpp"
#include "chatutils.hpp"
#include "json.hpp"
#include "loguru.hpp"
#include "lpi.h"
#include "string_algorithms.hpp"
#include <experimental/filesystem>
#include <fstream>
namespace fs = std::experimental::filesystem;
using json = nlohmann::json;

static constexpr int kMaxBufSize = 1024;

std::vector<Server> ParseConfig(const std::string &config) {
  fs::path cwd(fs::current_path());
  LOG_F(INFO, "cwd, path={%s}", cwd.c_str());
  fs::path config_file = cwd / config;
  if (!fs::exists(config_file)) {
    throw std::invalid_argument("Config doesn't exist.");
  }

  std::vector<Server> servers;
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
    std::string forward_addr, binding_addr;
    std::tie(forward_addr, binding_addr) = GetServerAddress(line);

    // Parse Addr and port and add to server list
    try {
      const auto forward = ParseAddress(forward_addr);
      const auto binding = ParseAddress(binding_addr);
      servers.emplace_back(forward, binding);
      LOG_F(INFO, "forward={%s}, binding={%s}", forward.addr().c_str(),
            binding.addr().c_str());
    } catch (const std::invalid_argument &err) {
      LOG_F(ERROR, "%s", err.what());
      continue;
    }
  }

  return servers;
}

ServerNode::ServerNode(int id, const std::string &order) : id_(id) {
  if (order == "unordered") {
    order_ = Order::UNORDERD;
    LOG_F(INFO, "unordered");
  } else if (order == "fifo") {
    order_ = Order::FIFO;
    LOG_F(INFO, "fifo order");
  } else if (order == "total") {
    order_ = Order::TOTAL;
    LOG_F(INFO, "total order");
  } else {
    order_ = Order::UNORDERD;
    LOG_F(WARNING, "invalid order, default to unordered");
  }
}

void ServerNode::Init(const std::string &config) {
  ReadConfig(config);
  SetupConnection();
}

void ServerNode::SetupConnection() {
  // Setup connection
  const Address &bind_addr = servers_[index()].bind_addr();
  fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_ == -1) {
    const auto msg = "Failed to create socket";
    LOG_F(ERROR, "%s", msg);
    errExit(msg);
  }

  // Bind to binding address
  auto serv_addr = MakeSockAddrInet(bind_addr);
  if (bind(fd_, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    const auto msg = "Failed to bind to address";
    LOG_F(ERROR, "%s", msg);
    errExit(msg);
  }

  LOG_F(INFO, "[S%d] Server addr={%s}", id(), bind_addr.addr().c_str());
}

void ServerNode::ReadConfig(const std::string &config) {
  try {
    servers_ = ParseConfig(config);
  } catch (const std::invalid_argument &err) {
    LOG_F(ERROR, "%s", err.what());
    errExit(err.what());
  }
  LOG_F(INFO, "Total servers, n={%zu}", n_servers());
}

void ServerNode::Run() {
  while (true) {
    Address addr;
    std::string msg;
    if (!RecvFrom(addr, msg)) {
      continue;
    }

    if (IsFromServer(addr)) {
      LOG_F(INFO, "[S%d] Msg from server, addr={%s}", id(),
            addr.addr().c_str());
      HandleServerMsg(addr, msg);
    } else {
      LOG_F(INFO, "[S%d] Msg from client, addr={%s}", id(),
            addr.addr().c_str());
      HandleClientMsg(addr, msg);
    }
  }
}

int ServerNode::GetServerIndex(const Address &addr) const {
  auto cmp_addr = [&](const Server &srv) { return srv.bind_addr() == addr; };
  auto it = std::find_if(servers_.begin(), servers_.end(), cmp_addr);
  if (it == servers_.end()) {
    return -1;
  } else {
    return std::distance(servers_.begin(), it);
  }
}

int ServerNode::GetClientIndex(const Address &addr) const {
  auto cmp_addr = [&](const Client &client) { return client.addr() == addr; };
  auto it = std::find_if(clients_.begin(), clients_.end(), cmp_addr);
  if (it == clients_.end()) {
    return -1;
  } else {
    return std::distance(clients_.begin(), it);
  }
}

void ServerNode::HandleServerMsg(const Address &addr, const std::string &msg) {
  auto j = json::parse(msg);
  Message m;
  m.nick = j["nick"];
  m.room = j["room"];
  m.text = j["text"];

  LOG_F(INFO, "[S%d] Parse msg, nick={%s}, room={%d}, msg={%s}", id(),
        m.nick.c_str(), m.room, m.text.c_str());

  if (order_ == Order::UNORDERD) {
    Deliver(m.room, m.Full());
  } else if (order_ == Order::FIFO) {
    m.seq = j["seq"];
    m.addr = j["addr"];
    // Put message in holdback queue
    hbq_fifo_.AddMessage(m);
    // Get the expected seq number of this message
    // expected_seq will be zero if it doesn't exist
    int &exp_seq = seq_fifo_[m.addr][m.room];
    LOG_F(INFO, "[S%d] Expected seq={%d}, sender={%s}, room={%d}", id(),
          exp_seq, m.addr.c_str(), m.room);
    // If there is a message in holdback queue that matches addr, room and seq
    // We will deliver that and increment exp_seq
    int msg_index;
    while ((msg_index = hbq_fifo_.GetMsgIndex(m.addr, m.room, exp_seq)) >= 0) {
      const auto &msg_td = hbq_fifo_.Get(msg_index);
      LOG_F(INFO, "[S%d] next message to deliver, seq={%d}", id(), msg_td.seq);
      Deliver(m.room, msg_td.Full());
      hbq_fifo_.RemoveByIndex(msg_index);
      LOG_F(INFO, "[S%d] queue size, n={%zu}", id(), hbq_fifo_.size());
      ++exp_seq;
    }
  } else if (order_ == Order::TOTAL) {
    // Total order
    m.id = j["id"];
    m.addr = j["addr"];
    const MsgType msg_type = j["type"];
    LOG_F(INFO, "[S%d] id={%d}, addr={%s}", id(), m.id, m.addr.c_str());

    if (msg_type == MsgType::NORMAL) {
      LOG_F(INFO, "[S%d] Got NORMAL msg", id());
      // This is a message from the first multicast
      // Each recipient responds with its proposed number
      // Pg_new = max(Pg_old, Ag) + 1 and then put (m, Pg_new) into their
      // local holdback queue, but marked as undeliverable
      m.seq = totalorder_.NewProposed(m.room);
      m.status = DeliverStatus::NOTDELIVERABLE;
      if (addr == servers_[index()].fwd_addr()) {
        LOG_F(INFO, "[S%d] From the same server, queue size={%zu}", id(),
              totalorder_.QueueSize(m.room));
        // This is from the same server
        Message &msg_iq = totalorder_.GetMessage(m.addr, m.room, m.id);
        // Just update seq number
        msg_iq.seq = m.seq;
        msg_iq.status = m.status;
      } else {
        // This is from another server, put in holdback queue
        totalorder_.AddMessage(m);
      }
      LOG_F(INFO, "[S%d] hbq size, n={%zu}", id(),
            totalorder_.QueueSize(m.room));

      // Build proposed message
      json jp;
      jp["nick"] = m.nick;
      jp["text"] = m.text;
      jp["room"] = m.room;
      jp["addr"] = m.addr;
      jp["seq"] = m.seq; // seq will be used in PROPOSE and DELIVER phases
      jp["type"] = MsgType::PROPOSE;
      jp["id"] = m.id;

      // Send back to original server
      SendTo(addr, jp.dump());
    } else if (msg_type == MsgType::PROPOSE) {
      m.seq = j["seq"];
      LOG_F(INFO, "[S%d] Got PROPOSE msg", id());
      // At this point message has to be in local queue
      // Find the same message in holdback queue
      Message &msg_iq = totalorder_.GetMessage(m.addr, m.room, m.id);
      LOG_F(INFO, "[S%d] msg addr={%s}, seq={%d}, room={%d}, id={%d}", id(),
            msg_iq.addr.c_str(), msg_iq.seq, msg_iq.room, msg_iq.id);
      // Add proposed seq number
      msg_iq.proposed.push_back(m.seq);
      LOG_F(INFO, "[S%d] add one proposed seq to message", id());

      // Check if it reaches the required amount of proposal to make a decision
      if (msg_iq.proposed.size() == n_servers()) {
        LOG_F(INFO, "[S%d] Got all proposal, addr={%s}, id={%d}, nick={%s}",
              id(), msg_iq.addr.c_str(), msg_iq.id, msg_iq.nick.c_str());

        // Build deliver message
        json jd;
        jd["id"] = msg_iq.id;
        jd["seq"] = msg_iq.MaxProposed();
        jd["nick"] = msg_iq.nick;
        jd["text"] = msg_iq.text;
        jd["room"] = msg_iq.room;
        jd["addr"] = msg_iq.addr;
        jd["type"] = MsgType::DELIVER;

        Multicast(jd.dump());
      }
      LOG_F(INFO, "HERE end of PROPOSE");
    } else if (msg_type == MsgType::DELIVER) {
      m.seq = j["seq"];
      LOG_F(INFO, "[S%d] Got DELIVER msg", id());

      // Upon receiving a (m, Tm) tuple, the recipients update m's number to Tm
      // and mark it as deliverable, and update its Ag_new = max(Ag_old, Tm)
      Message &msg_iq = totalorder_.GetMessage(m.addr, m.room, m.id);
      msg_iq.seq = m.seq;
      msg_iq.status = DeliverStatus::DELIVERBALE;
      totalorder_.UpdateAgreed(msg_iq.room, msg_iq.seq);
      LOG_F(INFO, "[S%d] Update agreed number, room={%d}, seq={%d}", id(),
            msg_iq.room, msg_iq.seq);

      // Sort message in queue by seq number
      totalorder_.SortQueue(msg_iq.room);

      // Now go through queue and deliever messages in order, if marked as
      // deliverable, then deliver and mark as delivered
      auto &msg_queue = totalorder_.GetQueue(msg_iq.room);
      for (Message &mq : msg_queue) {
        // If the first one is not deliverable, just break
        if (mq.status == DeliverStatus::NOTDELIVERABLE) {
          LOG_F(WARNING, "[S%d] Undeliverable msg in front, wait");
          break;
        }

        CHECK_F(mq.status != DeliverStatus::DELIVERED);

        if (mq.status == DeliverStatus::DELIVERBALE) {
          // Otherwise this one is deliverable so we deliver it
          Deliver(mq.room, mq.Full());
          // Then mark it as delivered
          mq.status = DeliverStatus::DELIVERED;
        }
      }

      // Afterward, we remove all delivered message in queue
      totalorder_.RemoveDelivered(msg_iq.room);
    }
  }
}

void ServerNode::Deliver(int room, const std::string &msg) const {
  for (const Client &client : clients_) {
    if (client.InRoom(room)) {
      SendMsgToClient(client, msg);
    }
  }
}

void ServerNode::HandleClientMsg(const Address &addr, const std::string &msg) {
  if (msg.empty()) {
    LOG_F(WARNING, "empty string");
    return;
  }

  // Try to get index
  auto client_index = GetClientIndex(addr);
  if (client_index < 0) {
    clients_.emplace_back(addr);
    client_index = clients_.size() - 1;
    LOG_F(INFO, "[S%d] Add a new client, addr={%s}", id(), addr.addr().c_str());
  }
  Client &client = clients_[client_index];

  // Check if the first character is a /
  if (msg[0] == '/') {
    // This is a command
    const auto cmd = trim_copy(msg.substr(1, 5));
    if (cmd == "join") {
      const auto room = msg.substr(6);
      Join(client, room);
    } else if (cmd == "nick") {
      const auto nick = msg.substr(6);
      Nick(client, nick);
    } else if (cmd == "part") {
      Part(client);
    } else if (cmd == "quit") {
      Quit(client);
    } else {
      LOG_F(WARNING, "[S%d] invalid command cmd={%s}", id(), cmd.c_str());
      ReplyErr(addr, "You entered an invalid command.");
    }
    LOG_F(INFO, "[S%d] Num clients, n={%zu}", id(), n_clients());
  } else {
    // No-op if client is not in a room
    if (!client.InRoom()) {
      LOG_F(WARNING, "[S%d] client no in room, client={%s}", id(),
            client.addr_str().c_str());
      return;
    }

    // Put message into json
    json j;
    j["nick"] = client.nick();
    j["room"] = client.room();
    j["text"] = msg;

    if (order_ == Order::FIFO) {
      j["seq"] = client.IncSeq();
      j["addr"] = client.addr_str();
    }

    if (order_ == Order::TOTAL) {
      j["type"] = MsgType::NORMAL;
      j["addr"] = client.addr_str();
      // We need id to uniquely identify a message with addr, room and id
      j["id"] = client.id;
      // Hack so that server put message into hbq immediately
      Message m;
      m.id = client.id;
      m.addr = client.addr_str();
      m.nick = client.nick();
      m.text = msg;
      m.room = client.room();
      m.status = DeliverStatus::NOTDELIVERABLE;
      totalorder_.AddMessage(m);
      LOG_F(INFO, "[S%d] Send out NORMAL msg, queue size={%zu}", id(),
            totalorder_.QueueSize(m.room));
      client.id++;
    }

    // Serialize and do simple multicast
    Multicast(j.dump());
  }
}

void ServerNode::Multicast(const std::string &msg) const {
  for (const Server &server : servers_) {
    SendTo(server.fwd_addr(), msg);
    LOG_F(INFO, "[S%d] Send to forward, addr={%s}", id(),
          server.fwd_addr().addr().c_str());
  }
}

bool ServerNode::SendTo(const Address &addr, const std::string &msg) const {
  auto dest = MakeSockAddrInet(addr);
  auto nsend = sendto(fd_, msg.c_str(), msg.size(), 0, (struct sockaddr *)&dest,
                      sizeof(dest));
  if (nsend < 0) {
    LOG_F(ERROR, "[S%d] Send failed, dest={%s}", id(), addr.addr().c_str());
    return false;
  } else if (nsend != static_cast<int>(msg.size())) {
    LOG_F(WARNING,
          "[S%d] Byte sent doesn't match msg size, nsend={%zu}, nmsg={%zu}",
          id_, nsend, msg.size());
    return false;
  } else {
    LOG_F(INFO, "[S%d] Msg sent, str={%s}, addr={%s}", id(), msg.c_str(),
          addr.addr().c_str());
    return true;
  }
}

bool ServerNode::RecvFrom(Address &addr, std::string &msg) const {
  struct sockaddr_in src;
  socklen_t srclen = sizeof(src);
  char buffer[kMaxBufSize];
  int nrecv = recvfrom(fd_, buffer, sizeof(buffer) - 1, 0,
                       (struct sockaddr *)&src, &srclen);
  if (nrecv < 0) {
    LOG_F(ERROR, "[S%d] recvfrom failed", id());
    return false;
  }

  // Null terminate buffer
  buffer[nrecv] = 0;
  msg = std::string(buffer, nrecv);
  addr = MakeAddress(src);
  LOG_F(INFO, "[S%d] Read, str={%s}, n={%d}, addr={%s}", id(), msg.c_str(),
        nrecv, addr.addr().c_str());
  return true;
}

void ServerNode::ReplyOk(const Address &addr, const std::string &msg) const {
  SendTo(addr, "+OK " + msg);
}

void ServerNode::ReplyErr(const Address &addr, const std::string &msg) const {
  SendTo(addr, "-ERR " + msg);
}

void ServerNode::ReplyOk(const Client &client, const std::string &msg) const {
  ReplyOk(client.addr(), msg);
}

void ServerNode::ReplyErr(const Client &client, const std::string &msg) const {
  ReplyErr(client.addr(), msg);
}

void ServerNode::Join(Client &client, const std::string &arg) {
  const int room = std::atoi(arg.c_str());
  if (room <= 0) {
    LOG_F(ERROR, "[S%d] Invalid room number, arg={%s}", id(), arg.c_str());
    ReplyErr(client, "You provided an invalid room number #" + arg);
    return;
  }

  if (client.InRoom()) {
    LOG_F(WARNING, "[S%d] client already in room, nick={%s}, room={%d}", id(),
          client.nick().c_str(), client.room());
    // If already in room reply with error message
    ReplyErr(client, "You are already in chat room #" + client.room_str());
  } else {
    // If not, join room
    client.JoinRoom(room);
    LOG_F(INFO, "[S%d] client join room, nick={%s}, room={%d}", id(),
          client.nick().c_str(), client.room());
    ReplyOk(client, "You are now in chat room #" + arg);
  }
}

void ServerNode::Nick(Client &client, const std::string &arg) {
  if (arg.empty()) {
    LOG_F(ERROR, "[S%d] No nick name provided", id());
    ReplyErr(client, "You provided an empty nick name");
    return;
  }

  client.set_nick(arg);
  LOG_F(INFO, "[S%d] Client changed nick, nick={%s}", id(),
        client.nick().c_str());
  ReplyOk(client, "Nick name set to '" + arg + "'");
}

void ServerNode::Part(Client &client) {
  const auto old_room = client.LeaveRoom();
  if (old_room < 0) {
    LOG_F(WARNING, "[S%d] Client is not in a room, nick={%s}", id(),
          client.nick().c_str());
    ReplyErr(client, "You are not in any room");
  } else {
    LOG_F(INFO, "[S%d] Client left room, nick={%s}, room={%d}", id(),
          client.nick().c_str(), old_room);
    ReplyOk(client, "You have left chat room #" + std::to_string(old_room));
  }
}

void ServerNode::Quit(Client &client) {
  LOG_F(INFO, "[S%d] Removing client, addr={%s}", id(),
        client.addr_str().c_str());
  // Remove client
  clients_.erase(std::remove(clients_.begin(), clients_.end(), client),
                 clients_.end());
}

bool ServerNode::IsFromServer(const Address &addr) const {
  auto cmp_addr = [&](const Server &srv) { return srv.fwd_addr() == addr; };
  auto it = std::find_if(servers_.begin(), servers_.end(), cmp_addr);
  return it != servers_.end();
}

void ServerNode::SendMsgToClient(const Client &client,
                                 const std::string &msg) const {
  SendTo(client.addr(), msg);
}
