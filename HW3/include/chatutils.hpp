#ifndef CHATUTILS_H
#define CHATUTILS_H

#include "addrport.hpp"
#include <arpa/inet.h>
#include <regex>
#include <string>

AddrPort ParseAddrPort(const std::string &addr_port);
std::tuple<std::string, std::string> GetForwardBinding(const std::string &line);

sockaddr_in MakeSockAddrInet(const AddrPort &addr_port);
sockaddr_in MakeSockAddrInet(const std::string &addr, int port);
AddrPort GetAddrPort(const sockaddr_in &sock_addr);

#endif // CHATUTILS_H
