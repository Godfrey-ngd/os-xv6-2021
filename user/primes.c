#pragma GCC diagnostic ignored "-Winfinite-recursion"

// user/primes.c
// 用“进程 + 管道”实现素数筛：每层过滤掉能被第一个数整除的数字。

#include "kernel/types.h"
#include "user/user.h"

static void
primeproc(int left[2]) {
  close(left[1]);           // 只读上游
  int p;
  // 读到的第一个数就是本层的质数；若读不到，说明结束
  if (read(left[0], &p, sizeof(p)) != sizeof(p)) {
    close(left[0]);
    exit(0);
  }
  printf("prime %d\n", p);

  // 为下一层创建管道
  int right[2];
  if (pipe(right) < 0) {
    fprintf(2, "primes: pipe failed\n");
    close(left[0]);
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: fork failed\n");
    close(left[0]);
    close(right[0]); close(right[1]);
    exit(1);
  }

  if (pid == 0) {
    // 子进程：递归处理下一层
    close(right[1]);        // 只读下游
    primeproc(right);       // 递归
    // 不返回
  } else {
    // 父进程：从上游读取，过滤后写入下游
    close(right[0]);        // 只写下游
    int x;
    while (read(left[0], &x, sizeof(x)) == sizeof(x)) {
      if (x % p != 0) {
        if (write(right[1], &x, sizeof(x)) != sizeof(x)) {
          fprintf(2, "primes: write failed\n");
          break;
        }
      }
    }
    // 关闭并收尾
    close(left[0]);
    close(right[1]);        // 关写端，通知下游 EOF
    wait(0);                // 等下一层退出
    exit(0);
  }
}

int
main(void) {
  int p[2];
  if (pipe(p) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    // 子进程：启动筛选管线
    primeproc(p);           // 不返回
  } else {
    // 父进程：生成 2..35 并写入起始管道
    close(p[0]);            // 只写
    for (int i = 2; i <= 35; i++) {
      if (write(p[1], &i, sizeof(i)) != sizeof(i)) {
        fprintf(2, "primes: write failed\n");
        break;
      }
    }
    close(p[1]);            // 关写端 -> 下游读到 EOF
    wait(0);
    exit(0);
  }
}
