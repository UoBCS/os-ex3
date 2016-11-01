
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char** argv){
  //usage
  if(argc != 2){
    fprintf(stderr, "usage: %s <string>\n", argv[0]);
    return -1;
  }

  //open the driver
  int f = open("/dev/opsysmem", O_WRONLY);
  if(f < 0){
    fprintf(stderr, "ERROR: cannot open /dev/opsysmem\n");
    return -1;
  }

  //write to it and return error codes
  int n = write(f, argv[1], strlen(argv[1]));
  if(n < 0){
    fprintf(stderr, "ERROR: write failed\n");
    close(f);
    return n;
  }

  //clean up
  close(f);

  return 0;
}

