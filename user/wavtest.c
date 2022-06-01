#include "user/user.h"
#include "user/wav.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(2, "Usage: %s [filename]\n", argv[0]);
    exit(1);
  }

  struct WavInfo wh;
  getHead(argv[1], &wh);
  getData(argv[1], &wh);
  fprintf(2, "chunk_id: %c%c%c%c\n", wh.chunk_id[0], wh.chunk_id[1],
          wh.chunk_id[2], wh.chunk_id[3]);

  exit(0);
}