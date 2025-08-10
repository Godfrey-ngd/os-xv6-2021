#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(2, "usage: trace mask command [args...]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);
  if (trace(mask) < 0) {
    fprintf(2, "trace: set mask failed\n");
    exit(1);
  }

  // 直接复用原 argv：argv[2] 是程序名，argv+2 指向以 0 结尾的参数数组
  exec(argv[2], argv + 2);

  // 只有 exec 失败才会走到这里
  fprintf(2, "trace: exec %s failed\n", argv[2]);
  exit(1);
}

