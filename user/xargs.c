#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAX_LINE_LENGTH 512

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command> [args...]\n");
        exit(1);
    }

    char *command = argv[1];
    char *args[MAXARG];
    int i;
    
    // 初始化参数数组，保留原始命令参数
    for (i = 1; i < argc; i++) {
        args[i-1] = argv[i];
    }

    char line[MAX_LINE_LENGTH];
    char c;
    int pos = 0;

    // 逐字符读取标准输入
    while (read(0, &c, 1) > 0) {
        if (c == '\n') {
            // 遇到换行符，处理完整的一行
            line[pos] = '\0';
            pos = 0;
            
            // 添加输入行作为最后一个参数
            args[argc-1] = line;
            args[argc] = 0; // 参数数组必须以0结尾
            
            // 创建子进程执行命令
            int pid = fork();
            if (pid == 0) {
                // 子进程
                exec(command, args);
                fprintf(2, "xargs: exec %s failed\n", command);
                exit(1);
            } else {
                // 父进程等待子进程完成
                wait(0);
            }
        } else if (pos < MAX_LINE_LENGTH - 1) {
            // 存储字符到行缓冲区
            line[pos++] = c;
        } else {
            // 行太长，跳过剩余部分
            fprintf(2, "xargs: line too long\n");
            while (read(0, &c, 1) > 0 && c != '\n');
            pos = 0;
        }
    }

    exit(0);
}