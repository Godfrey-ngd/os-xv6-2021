#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: time command [args...]\n");
    exit(1);
  }
  int start = uptime();
  int pid = fork();
  if (pid < 0) { fprintf(2, "time: fork failed\n"); exit(1); }
  if (pid == 0) {
    exec(argv[1], argv+1);
    fprintf(2, "time: exec %s failed\n", argv[1]);
    exit(1);
  }
  int x = 0; wait(&x);
  int end = uptime();
  printf("elapsed %d ticks\n", end - start);   // ★ stdout
  exit(0);
}

