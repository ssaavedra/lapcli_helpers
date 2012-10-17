#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef BRIGHTFILE
#define BRIGHTFILE "/sys/class/backlight/gmux_backlight/brightness"
#endif

#ifndef MAX
#define MAX(a, b) (a<b?b:a)
#endif

int brightfd;

int read_bright()
{
  char buffer[5];
  ssize_t s;
  s = pread(brightfd, buffer, 5, 0);
  if(s < 0) {
    perror("read_bright");
    exit(1);
  }
  return atoi(buffer);
}

int write_bright(int new)
{
  char buffer[5];
  ssize_t s;
  s = sprintf(buffer, "%04d\n", new);
  if(s < 0) {
    perror("write_bright");
    exit(1);
  }
  pwrite(brightfd, buffer, 5, 0);
}

int fancy_rebright(int target_bright)
{
  int i;
  int old_bright = read_bright();
  int iterations = old_bright - target_bright;
  iterations /= 50;

  if(iterations < 0) {
    iterations = 0 - iterations;
    for(i = 0; i < iterations; i++) {
      write_bright(old_bright + 50 * i);
      usleep(5000);
    }
  } else {
    for(i = 0; i < iterations; i++) {
      write_bright(old_bright - 50 * i);
      usleep(5000);
    }
  }
}

int main(int argc, char *argv[])
{
  char buffer[10];
  int nofancy = 0, desired, c;
  enum predef { DO_MORE, DO_LESS, DO_INT, DO_DONE, DO_QUERY } predef = DO_INT;

  brightfd = open(BRIGHTFILE, O_RDWR);
  if(brightfd < 1) {
    perror("open");
    exit(1);
  }

  while(-1 != (c = getopt(argc, argv, "fmlq"))) {
    switch(c)
      {
      case 'f':
	nofancy = 1;
	break;
      case 'm':
	predef = DO_MORE;
	break;
      case 'l':
	predef = DO_LESS;
	break;
      case 'q':
	predef = DO_QUERY;
      }
  }

  if(argv[optind] != NULL) {
    predef = DO_DONE;
    desired = atoi(argv[optind]);
  }

  switch(predef)
    {
    case DO_QUERY:
      printf("Current brightness: %d\n", read_bright());
      return 0;
    case DO_INT:
      printf("Desired brightness [0-16000]: ");
      fgets(buffer, 8, stdin);
      desired = atoi(buffer);
      break;
    case DO_MORE:
      desired = MAX(0, read_bright() + 500);
      break;
    case DO_LESS:
      desired = MAX(0, read_bright() - 500);
    case DO_DONE: default: break;
    }
  

  
  if(nofancy)
    write_bright(desired);
  fancy_rebright(desired);
  write_bright(desired);

  return 0;
}


