#ifndef CHATUTILS_H
#define CHATUTILS_H

#include "address.hpp"
#include <arpa/inet.h>
#include <regex>
#include <string>

/**
 * @brief ParseAddress Generate address from string
 * @param addr_str String in the form of addr:port
 * @return an Address instance
 */
Address ParseAddress(const std::string &addr_str);

/**
 * @brief GetServerAddress
 * @param line
 * @return
 */
std::tuple<std::string, std::string> GetServerAddress(const std::string &line);

/**
 * @brief MakeSockAddrInet Make sockaddr_in from Address
 * @param addr
 * @return
 */
sockaddr_in MakeSockAddrInet(const Address &addr);
sockaddr_in MakeSockAddrInet(const std::string &ip, int port);

/**
 * @brief GetAddress Make address from sockaddr_in
 * @param sock_addr
 * @return
 */
Address MakeAddress(const sockaddr_in &sock_addr);

#endif // CHATUTILS_H
