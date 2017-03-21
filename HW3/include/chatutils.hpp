#ifndef CHATUTILS_H
#define CHATUTILS_H

#include <regex>
#include <string>

/**
 * @brief ParseAddrAndPort
 * @param addr_port String supposedly in the form addr:port
 * @return (addr, port)
 */
std::tuple<std::string, int> ParseAddrAndPort(const std::string &addr_port);

#endif // CHATUTILS_H
