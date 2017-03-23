#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
public:
  explicit Server(int index);

  bool ReadConfig(const std::string &config);
  void Run();

private:
  int index_;
};

#endif // SERVER_HPP
