#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include <stddef.h>
#define STDIN 0
#define STDOUT 1
#define MAXARGLEN 32

char* myStrdup(const char* s) {
    if (s == NULL) return NULL;
    char* new_str = malloc(strlen(s) + 1);  // 加 1 为 null 终结符分配空间
    if (new_str == NULL) return NULL;
    strcpy(new_str, s);  // 复制字符串包括 null 终结符
    return new_str;
}


void allocForArgs(char *args[], int argNum, int argLen)
{
    for (int i = 0; i < argNum; i++) {
        args[i] = malloc(argLen * sizeof(char));
        if (args[i] == 0) {
            printf("Memory allocation failed for argument %d\n");
            // 如果分配失败，释放之前已分配的内存并退出
            while (i-- > 0) {
                free(args[i]);
            }
            exit(1);
        }
    }
}

void freeArgs(char *args[], int argNum)
{
    for (int i = 0; i < argNum; i++) {
        free(args[i]);
        args[i] = NULL;  // 避免悬空指针
    }
}


int main(int argc, char *argv[])
{
    
    // 确保至少提供了一个命令
    if (argc < 2) {
        fprintf(STDOUT, "Usage: %s <command> [args]\n");
        exit(1);
    }

    char *args[MAXARG];  // 存储命令和参数的数组
    allocForArgs(args, MAXARG, MAXARGLEN);  // 为参数数组分配内存
    int argNum = 0;  // 参数索引
    char buffer[MAXARGLEN];  // 用于临时存储标准输入的数据
    int bufferLen = 0;  // 缓冲区当前长度

    // 处理argv中的命令和参数
    for (int i = 1; i < argc && argNum < MAXARG; i++, argNum++) {
        strcpy(args[argNum], argv[i]);
        args[argNum][MAXARGLEN - 1] = '\0';  // 确保字符串结束
    }

    // 从标准输入读取额外的参数
    char ch;
    while (read(STDIN, &ch, 1) > 0 && argNum < MAXARG) {
        if (ch == '\n' || ch == ' ') {
            if (bufferLen > 0) {
                args[argNum] = myStrdup(buffer);  // 复制缓冲区到args
                bufferLen = 0;  // 重置缓冲区长度
                if (++argNum >= MAXARG) break;  // 检查参数数量限制
            }
        } else {
            if (bufferLen < MAXARGLEN - 1) {
                buffer[bufferLen++] = ch;
                buffer[bufferLen] = '\0';  // 保持buffer为有效的C字符串
            }
        }
    }

    // 设置参数数组的结束标志
    args[argNum] = 0;

    // 创建子进程执行命令
    if (fork() == 0) {
        exec(args[0], args); 
        freeArgs(args, argNum);
    } else {
        wait(0);  // 父进程等待子进程结束
        freeArgs(args, argNum);  // 释放分配的内存
    }

    return 0;
}