#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int count = getprocs();
  printf("There are %d active processes.\n", count);
  int pid = fork();
  int count1 = getprocs();
  if(pid!=0){
    wait(0);
    printf("Now, there are %d active processes.\n", count1);
  }
  exit(0);
}