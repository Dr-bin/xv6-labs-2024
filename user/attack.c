#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int strncmp(const char *s1, const char *s2, int n) {
    for(int i = 0; i < n; i++) {
        if(s1[i] != s2[i])
            return s1[i] - s2[i];
        if(s1[i] == '\0')
            return 0;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
  // your code here.  you should write the secret to fd 2 using write
  // (e.g., write(2, secret, 8)

  char *end = sbrk(PGSIZE*32);

  char* secret;

  for(int i = 0; i < 32; i++){
    char* page = end + i * PGSIZE;
    if(strncmp(page+18, "secret pw ", 10) == 0){
      secret = page+32;
      write(2, secret, 8);
      exit(0);
    }
  }

  write(2, "No secret found\n", 16);
  exit(1);
}
