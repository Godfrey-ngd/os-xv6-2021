#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  int p2c[2]; // parent -> child
  int c2p[2]; // child  -> parent
  char byte = 'x';
  char buf;

  // 创建两个管道，每个方向一个
  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // --- 子进程 ---
    // 关闭不会用到的端口：对子进程而言，只读 p2c[0]，只写 c2p[1]
    close(p2c[1]); // 不往 p2c 写
    close(c2p[0]); // 不从 c2p 读

    // 从父到子的管道读一个字节
    if (read(p2c[0], &buf, 1) != 1) {
      fprintf(2, "pingpong: child read failed\n");
      exit(1);
    }
    // 打印“收到 ping”，这里 pid 是子进程自己的 pid
    printf("%d: received ping\n", getpid());

    // 回写一个字节到子->父的管道
    if (write(c2p[1], &byte, 1) != 1) {
      fprintf(2, "pingpong: child write failed\n");
      exit(1);
    }

    // 清理并退出
    close(p2c[0]);
    close(c2p[1]);
    exit(0);
  } else {
    // --- 父进程 ---
    // 关闭不会用到的端口：对父而言，只写 p2c[1]，只读 c2p[0]
    close(p2c[0]); // 不从 p2c 读
    close(c2p[1]); // 不往 c2p 写

    // 先给子进程发一个字节
    if (write(p2c[1], &byte, 1) != 1) {
      fprintf(2, "pingpong: parent write failed\n");
      // 继续收尾
    }
    close(p2c[1]); // 写完就关写端，避免子进程阻塞等待读EOF

    // 从子进程读回一个字节
    if (read(c2p[0], &buf, 1) != 1) {
      fprintf(2, "pingpong: parent read failed\n");
      // 继续收尾
    }
    printf("%d: received pong\n", getpid());

    close(c2p[0]);
    wait(0);       // 等子进程退出
    exit(0);
  }
}
