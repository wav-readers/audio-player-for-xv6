/*#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2 || atoi(argv[1]) < 0 || atoi(argv[1]) > atoi(argv[2])) {
    printf("The second parameter should be >= 0 and <= the third parameter.\n");
    exit(0);
  }
  setaudiovolume(atoi(argv[1]), atoi(argv[2]));
  exit(0);
}*/
