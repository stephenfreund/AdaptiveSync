
#include "patch.h"
#include <sys/mman.h>
#include <unistd.h>

int set_permissions(void *addr, int writeable) {
  int page_size = getpagesize();
  addr = (void *)((unsigned long)addr - (unsigned long)addr % page_size);

  if(mprotect(addr, page_size, PROT_READ | (writeable != 0 ? PROT_WRITE : 0) | PROT_EXEC) == -1) {
    return -1;
  }

  return 0;
}

