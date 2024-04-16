#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  printf("Run zalloc show user\n");
  zallocshowuser();
  exit(0);
}