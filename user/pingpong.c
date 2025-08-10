#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
    char buf = 'p';

    int fd_c2p[2];
    int fd_p2c[2];

    pipe(fd_c2p);
    pipe(fd_p2c);

    int pid = fork();

    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        exit(1);
    }
    else if(pid == 0)//child
    {
        close(fd_c2p[1]);
        close(fd_p2c[0]);

        read(fd_c2p[0], &buf, 1);
        printf("%d: received ping\n", getpid());

        write(fd_p2c[1], &buf, 1);
        close(fd_c2p[0]);
        close(fd_p2c[1]);
        exit(0);
    }
    else//parent
    {
        close(fd_c2p[0]);
        close(fd_p2c[1]);

        write(fd_c2p[1], &buf, 1);
        read(fd_p2c[0], &buf, 1);
        printf("%d: received pong\n", getpid());

        close(fd_c2p[1]);
        close(fd_p2c[0]);
        exit(0);
    }
}