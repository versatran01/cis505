#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
  if (argc != 2) {
  	fprintf(stderr, "Syntax: %s howManyNumbers\n", argv[0]);
  	exit(1);
  }
  
  srand(time(0));
  for (int i=0; i<atoi(argv[1]); i++) {
  	long long v = rand();
  	for (int j=0; j<3; j++)
  	  v = (v<<16)+rand();
    printf("%lld\n", v);
  }

  return 0;
}