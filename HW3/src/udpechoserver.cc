#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main() {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  
  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
  serv_addr.sin_port = htons(4711);
  bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  
  while (true) {
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);
    char buf[100];
    int rlen = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&src,
                        &srclen);
    buf[rlen] = 0;
    
    printf("Echoing [%s] to %s\n", buf, inet_ntoa(src.sin_addr));
    sendto(sock, buf, rlen, 0, (struct sockaddr*)&src, sizeof(src));
  }

}


