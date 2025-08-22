#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"  // MAXARG
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "Usage: xargs <command> [args...]\n");
    exit(1);
  }

  // 固定参数（命令及其原有参数）拷到 base[]
  char *base[MAXARG];
  int basec = 0;
  for (int i = 1; i < argc; i++) {
    base[basec++] = argv[i];
  }

  char buf[1024];   // 累积当前行的动态参数内容
  int  bi = 0;      // 写入位置
  char *dyn[MAXARG]; // 当前行解析出的参数指针
  int  dync = 0;

  int in_tok = 0;       // 是否在 token 内
  int tok_start = -1;   // 当前 token 在 buf 中的起始下标

  char ch;
  int n;
  while ((n = read(0, &ch, 1)) == 1) {
    if (ch == ' ' || ch == '\t' || ch == '\n') {
      if (in_tok) {
        // 结束一个 token：补 0，并记录指针
        if (bi >= (int)sizeof(buf)) {
          fprintf(2, "xargs: line too long\n");
          exit(1);
        }
        buf[bi++] = 0;
        if (basec + dync + 1 >= MAXARG) {
          fprintf(2, "xargs: too many args\n");
          exit(1);
        }
        dyn[dync++] = &buf[tok_start];
        in_tok = 0;
      }
      if (ch == '\n') {
        // 一行结束 -> 执行一次
        if (dync > 0) {
          int pid = fork();
          if (pid == 0) {
            char *final[MAXARG];
            int j = 0;
            for (; j < basec; j++) final[j] = base[j];
            for (int k = 0; k < dync; k++) final[j++] = dyn[k];
            final[j] = 0;
            exec(final[0], final);
            fprintf(2, "xargs: exec %s failed\n", final[0]);
            exit(1);
          } else if (pid > 0) {
            wait(0);
          } else {
            fprintf(2, "xargs: fork failed\n");
            exit(1);
          }
        }
        // 重置为读取下一行
        bi = 0; dync = 0; in_tok = 0; tok_start = -1;
      }
    } else {
      if (bi >= (int)sizeof(buf) - 1) {
        fprintf(2, "xargs: line too long\n");
        exit(1);
      }
      if (!in_tok) { in_tok = 1; tok_start = bi; }
      buf[bi++] = ch;
    }
  }

  // EOF：若最后一行没有 '\n' 但仍有内容，也执行一次
  if (in_tok) {
    buf[bi++] = 0;
    if (basec + dync + 1 >= MAXARG) {
      fprintf(2, "xargs: too many args\n");
      exit(1);
    }
    dyn[dync++] = &buf[tok_start];
  }
  if (dync > 0) {
    int pid = fork();
    if (pid == 0) {
      char *final[MAXARG];
      int j = 0;
      for (; j < basec; j++) final[j] = base[j];
      for (int k = 0; k < dync; k++) final[j++] = dyn[k];
      final[j] = 0;
      exec(final[0], final);
      fprintf(2, "xargs: exec %s failed\n", final[0]);
      exit(1);
    } else if (pid > 0) {
      wait(0);
    } else {
      fprintf(2, "xargs: fork failed\n");
      exit(1);
    }
  }

  exit(0);
}
