#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

int main(int argc, char* argv[]) {
  
  uLong crc = crc32(0L, (Bytef*)argv[1], 5);

  printf("%d\n", (unsigned int)crc);
}
