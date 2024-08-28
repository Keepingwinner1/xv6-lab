#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define MAX_PATH_LENGTH 512

// 改造ls.c中的fmtname,使得返回的字符串后不自动补齐空格
char *fmtname(char *path)
{
    static char buf[DIRSIZ + 1];  // 静态缓冲区，用于存储和返回处理后的文件名
    char *p;

    // 寻找路径中最后一个斜杠后的第一个字符，即文件名的开始
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // 如果文件名的长度已经超过或等于 DIRSIZ，直接返回这个文件名
    if (strlen(p) >= DIRSIZ)
        return p;

    // 将文件名从 p 复制到 buf 中
    memmove(buf, p, strlen(p));

    // 在文件名后填充空格，直到达到 DIRSIZ 的长度
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));

    // 在填充后的字符串中从后向前搜索，找到第一个非空格字符
    for (char *i = buf + strlen(buf);; i--) {
        if (*i != '\0' && *i != ' ' && *i != '\n' && *i != '\r' && *i != '\t') {
            *(i + 1) = '\0';  // 在这个字符后面放置字符串结束符 '\0'
            break;
        }
    }

    return buf;  // 返回处理后的字符串
}




// 定义find函数，用于在指定目录及其子目录中搜索具有特定名称的文件，从ls函数改造而来
void find(char *path, char *filename)
{
    char buf[MAX_PATH_LENGTH];  // 用于存储完整的文件或目录路径
    char *p;  // 辅助指针，用于操作路径字符串
    int fd;  // 文件描述符，用于打开目录
    struct dirent de;  // 目录项结构
    struct stat st;  // 文件状态结构

    // 尝试打开目录或文件
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);  // 打开失败时输出错误信息
        return;
    }

    // 获取文件或目录的状态信息
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);  // 获取状态失败时输出错误信息
        close(fd);  // 关闭文件描述符
        return;
    }

    // 根据文件类型处理
    switch (st.type) {
    case T_FILE:  // 处理文件类型
        if (strcmp(fmtname(path), filename) == 0) {  // 比较处理后的文件名是否与目标文件名匹配
            printf("%s\n", path);  // 如果匹配，打印文件路径
        }
        break;

    case T_DIR:  // 处理目录类型
        // 检查路径长度是否超过缓冲区大小
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            fprintf(2, "find: path too long\n");  // 如果路径太长，输出错误信息
            break;
        }
        strcpy(buf, path);  // 将当前路径复制到缓冲区
        p = buf + strlen(buf);  // 设置指针到缓冲区末尾
        *p++ = '/';  // 在路径末尾添加斜杠，为拼接子目录或文件名做准备

        // 读取目录中的每个条目
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            // 跳过无效的目录项以及 "." 和 ".."
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                continue;
            }
            memmove(p, de.name, DIRSIZ);  // 将读取的目录项名称拼接到路径后
            p[DIRSIZ] = 0;  // 确保路径字符串正确结束
            if (stat(buf, &st) < 0) {
                fprintf(2, "find: cannot stat %s\n", buf);  // 如果无法获取子目录项的状态，输出错误信息并继续
                continue;
            }
            find(buf, filename);  // 递归调用find函数，以检查子目录
        }
        break;
    }

    close(fd);  // 完成后关闭文件描述符
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(2, "Usage: find <directory> <filename>\n");
        exit(0);
    }

    find(argv[1], argv[2]);
    exit(0);
}