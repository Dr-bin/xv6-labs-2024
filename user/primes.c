#include "kernel/types.h"
#include "user/user.h"

void transmit(int lp[2], int rp[2], int* first)
{
    int temp;
    while(read(lp[0], &temp, sizeof(int)) == sizeof(int))
    {
        if(temp % *first != 0)
            write(rp[1], &temp, sizeof(int));
    }
    close(lp[0]);
    close(rp[1]);
    return;
}

void prime(int lp[2])
{
    close(lp[1]);
    int first;
    
    if(read(lp[0], &first, sizeof(int)) == sizeof(int)){
    printf("prime %d\n", first);

    int rp[2];
    pipe(rp);
    transmit(lp, rp, &first);

    if (fork() == 0) {
        close(rp[1]);
        close(lp[0]);
        prime(rp);
    } else {
        close(rp[0]);
        wait(0);
    }
    }
}

int main(int argc, char const *argv[])
{
    int p[2];
    pipe(p);

    for(int i = 2; i <= 35; i++)
        write(p[1], &i, sizeof(int));

    if(fork() == 0){
        prime(p);
    }
    else{
        close(p[0]);
        close(p[1]);
        wait(0);
    }
    exit(0);
}