#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
  syscall(345,"exp1.c","exp2_test.c");
  return 0;
}
