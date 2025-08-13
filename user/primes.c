#include "kernel/types.h"
#include "user/user.h"

// 转发非素数到下一个管道
void transmit(int lp[2], int rp[2], int* first)
{
    int temp;
    while(read(lp[0], &temp, sizeof(int)) == sizeof(int))//要读左边
    {
        // fprintf(0,"%d %% %d\n", *first, temp);
        if(temp % *first != 0)
            write(rp[1], &temp, sizeof(int));
    }
    return;
}

// 素数筛进程（递归）
void prime(int lp[2])
{
    close(lp[1]);// 关左写
    int first;
    // 读取第一个数作为素数
    if(read(lp[0], &first, sizeof(int)) != sizeof(int)){
        close(lp[0]);
        return;
    }

    printf("prime %d\n", first);

    int rp[2];
    pipe(rp);

    if (fork() == 0) {
        close(rp[1]);// 关闭右管道写端
        close(lp[0]);// 关左读
        prime(rp);
    } else {
        close(rp[0]);// 关右读
        transmit(lp, rp, &first);// 此时读左管道写右管道
        close(rp[1]);// 关右写
        close(lp[0]);// 关左读
        wait(0);
        return;
    }
}

int main(int argc, char const *argv[])
{
    int p[2];
    pipe(p);

    if(fork() == 0){
        prime(p);
    }
    else{
        for(int i = 2; i <= 280; i++)
        {
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);
        close(p[0]);
        wait(0);
    }
    exit(0);
}