
#ifndef PATCH_H_
#define PATCH_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned char *jmp_offset_start;
  unsigned int offsets[10];
  int num_offsets;
} patch_t;

#define EXTERN(L) extern unsigned char *L
#define EXTERN3(L1, L2, L3) \
  __asm__(".globl " #L1 "\n\t"); \
  __asm__(".globl " #L2 "\n\t");		\
  __asm__(".globl " #L3 "\n\t");		\
  extern unsigned char *L1, *L2, *L3; 
  
#define JMP(JMP, L1, L2)	     \
  __asm__(".p2align 2\n\t"); \
 JMP: \
  __asm__(#JMP ": \n\t");\
  __asm__ goto (".byte 0xe9\n\t" \
		".long " #L1 " - (.+4)\n\t" \
		:::: L1, L2);			\

#define LABEL(L) \
  __asm__(".p2align 2\n\t"); \
 L: \
  __asm__(#L ": \n\t");

#define PATCH(MODE, JMP, L1, L2)			\
  { \
    int *off = (int *)(((unsigned char *)JMP)+1);	\
    /*printf("%p : \n", JMP);				\
      fflush(stdout); */						\
    int new_offset = (MODE == 0 ? ((unsigned char *)L1) : ((unsigned char *)L2)) - (((unsigned char *)JMP) + 5);\
    /*printf("%p : %X -> %X\n", JMP, *off, new_offset);			\
      fflush(stdout);*/							\
    *off = new_offset;\
  }

  int set_permissions(void *addr, int writeable);

  
#ifdef __cplusplus
}
#endif

#endif
