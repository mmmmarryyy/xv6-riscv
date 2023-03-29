#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"

void vmprint_pagetable(pagetable_t pagetable, int tree_depth) {
  for (int i = 0; i < 512; i++) { //512 - size of pagetable
 	pte_t pte = pagetable[i];

 	if ((pte & PTE_V)) {
 	  for (int repeat_ind = 0; repeat_ind < tree_depth; repeat_ind++) {
 		printf("..");
      }

 	  printf("%d: pte %p pa %p\n", i, pte, PTE2PA(pte));

 	  if ((pte & (PTE_R | PTE_W | PTE_X)) == 0) {
 		vmprint_pagetable((pagetable_t)PTE2PA(pte), tree_depth + 1);
      }
 	}
  }
}