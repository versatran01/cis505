#include <cstdio>
#include <iostream>
#include <thread>
#include <unistd.h>

struct Connection {
  int fd;
  ~Connection() {
    std::cout << std::this_thread::get_id() << ": destructor, fd: " << fd
              << std::endl;
    std::printf("%d: destructor", fd);
  }
};

void thread_fun(int fd) {
  Connection c;
  c.fd = fd;
  int i = 0;
  while (true) {
    usleep(100000);
    std::cout << fd << std::endl;
    i++;
    if (fd == 2 && i == 20) {
      return;
    }
  }
}

int main() {

  //  std::thread t1(thread_fun, 1);
  std::thread t3(thread_fun, 3);
  //  t1.detach();

  {
    std::thread t2(thread_fun, 2);
    t2.detach();
  }
  std::cout << "here" << std::endl;

  t3.join();
}
