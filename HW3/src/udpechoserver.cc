#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

/**
 * @brief Simple udp echoserver for testing
 */
int main(int argc, char **argv) {
  int port = 0;
  if (argc > 1) {
    port = std::atoi(argv[1]);
  }

  if (port < 1024) {
    fprintf(stderr, "port %d < 1024, use default 8000.\n", port);
    port = 8000;
  }

  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
  serv_addr.sin_port = htons(port);
  bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  while (true) {
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);
    char buf[1024];
    int rlen = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&src,
                        &srclen);
    buf[rlen] = 0;

    printf("Echoing [%s] to %s\n", buf, inet_ntoa(src.sin_addr));
    sendto(sock, buf, rlen, 0, (struct sockaddr *)&src, sizeof(src));
  }
}
