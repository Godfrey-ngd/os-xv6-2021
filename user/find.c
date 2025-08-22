#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 递归查找：在 path 下查找名为 target 的文件/目录
static void
find(const char *path, const char *target)
{
  char buf[512], *p;        // buf 保存拼接出来的子路径
  int fd;
  struct stat st;
  struct dirent de;

  // 打开 path
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type == T_FILE) {
    // 如果 path 本身就是一个文件，判断最后一段是否等于 target
    // 取出最后一个 '/' 之后的文件名部分
    const char *name = path;
    for (const char *q = path; *q; q++)
      if (*q == '/')
        name = q + 1;
    if (strcmp(name, target) == 0)
      printf("%s\n", path);
  } else if (st.type == T_DIR) {
    // 构造 "path/xxxxx"
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "find: path too long: %s\n", path);
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    // 枚举目录项
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;

      // 跳过 "." 和 ".."
      if (de.name[0] == '.' &&
          (de.name[1] == 0 || (de.name[1] == '.' && de.name[2] == 0)))
        continue;

      // de.name 是固定长度 DIRSIZ 的，不保证以 0 结尾；拷到 buf 并手动补 0
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (stat(buf, &st) < 0) {
        // stat 失败就跳过
        // fprintf(2, "find: cannot stat %s\n", buf);
        continue;
      }

      if (st.type == T_FILE) {
        // 比较文件名是否等于 target
        // 注意此时 p 指向的是名字开始处，且我们已补了 0
        if (strcmp(p, target) == 0)
          printf("%s\n", buf);
      } else if (st.type == T_DIR) {
        // 递归进入子目录
        find(buf, target);
      }
      // 下一轮循环会覆盖 p 开始处，无需回退
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(2, "Usage: find <path> <name>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
