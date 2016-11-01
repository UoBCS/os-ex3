
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char** argv){
  //usage
  if(argc != 2){
    fprintf(stderr, "usage: %s <length>\n", argv[0]);
    return -1;
  }

  //init
  errno = 0;
  long int len = strtol(argv[1], NULL, 10);
  if(errno != 0){
    perror("ERROR: strtol");
    return -1;
  }
  char* buf = malloc((size_t)len+1);
  if(buf == NULL){
    fprintf(stderr, "ERROR: malloc failed\n");
    return -1;
  }
  
  //open the driver
  int f = open("/dev/opsysmem", O_RDONLY);
  if(f < 0){
    fprintf(stderr, "ERROR: cannot open /dev/opsysmem\n");
    return -1;
  }

  //read and return error codes
  errno = 0;
  int n = read(f, buf, len);
  if(n < 0){
    perror("ERROR: read");
    close(f);
    return n;
  }
  buf[n] = '\0';

  //print
  printf("%s", buf);
  
  //clean up
  close(f);

  return 0;
}

