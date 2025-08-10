#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
    if(argc != 2){
        printf("wrong argc, use sleep <time>\n");
        exit(1);
    }
    sleep(atoi(argv[1]));
    exit(0);
}
