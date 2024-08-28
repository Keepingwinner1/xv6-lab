#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    // 检查程序是否有额外的命令行参数，如果有则输出错误并退出
    if (argc != 1){
        fprintf(2,"Error: No need for arguments...\n");
        exit(1);
    }

    int p[2];
    // 创建管道，用于进程间通信
    pipe(p);

    // 创建子进程
    if (fork() == 0){ // 子进程
        close(p[0]); // 关闭管道的读端，因为子进程只需要写

        char temp = 'x'; // 要写入管道的字符
        // 将字符写入管道并检查是否写入成功
        if (write(p[1],&temp,1))
            fprintf(0,"%d: received ping\n",getpid()); // 输出收到ping信息

        close(p[1]); // 关闭管道的写端
    }
    else{ // 父进程
        wait((int *)0); // 等待子进程完成

        close(p[1]); // 关闭管道的写端，因为父进程只需要读

        char temp;
        // 从管道中读取字符并检查是否读取成功
        if (read(p[0],&temp,1))
            fprintf(0,"%d: received pong\n",getpid()); // 输出收到pong信息

        close(p[0]); // 关闭管道的读端
    }

    // 正常退出程序
    exit(0);
}
