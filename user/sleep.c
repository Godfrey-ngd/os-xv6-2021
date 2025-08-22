// user/sleep.c
// 实现一个简单的 sleep 命令，让进程暂停指定 tick 数
// 在 xv6 中，sleep() 是内核提供的系统调用，参数是时钟 tick 数，不是秒

#include "kernel/types.h"  // 基本数据类型定义
#include "user/user.h"     // 用户态系统调用接口

int
main(int argc, char *argv[])
{
  // 检查参数数量，sleep 需要一个参数（暂停的 tick 数）
  if (argc != 2) {
    fprintf(2, "Usage: sleep <ticks>\n"); // 输出到标准错误
    exit(1);                              // 非正常退出
  }

  // 将字符串参数转为整数
  int ticks = atoi(argv[1]);
  if (ticks <= 0) {
    fprintf(2, "sleep: ticks must be positive\n");
    exit(1);
  }

  // 调用 xv6 的 sleep 系统调用，单位是 tick（约 1/10 秒）
  sleep(ticks);

  // 正常结束程序
  exit(0);
}
