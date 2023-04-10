#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"

void pgaccess(pagetable_t pagetable) {
  for (int i = 0; i < 512; i++) { //512 - size of pagetable
    pte_t pte = pagetable[i];
    
    if ((pte & PTE_V)) {
      if (pte & PTE_A) {
 	      printf("%d: pte %p pa %p\n", i, pte, PTE2PA(pte));
        pagetable[i] ^= PTE_A;
      }

 	    if ((pte & (PTE_R | PTE_W | PTE_X)) == 0) {
 		    pgaccess((pagetable_t)PTE2PA(pte));
      } 
 	  }
  }
}