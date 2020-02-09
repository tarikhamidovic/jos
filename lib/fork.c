// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
  // Provjeravamo error code
  if (!(err & FEC_WR))
    panic("pgfault(): write on non writeable");
  // Provjeravamo da li su page directory ili page prisutni
  if (!((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P))) 
    panic("pgfault(): page directory or page table not present");
  // Provjera da li je page copy-on-write
  if (!(uvpt[PGNUM(addr)] & PTE_COW)) 
    panic("pgfault(): not copy-on-write");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
  // Alociramo novu stranicu mapiranu na lokaciju PFTEMP
  if (sys_page_alloc(0, PFTEMP, PTE_W | PTE_U | PTE_P) < 0) 
    panic("pgfault(): sys_page_alloc failed");
  // Kopiramo sadrzaj sa stare u novu stranicu
  memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
  // Prebacujemo novu stranicu na adresu stare stranice
  if (sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W) < 0) 
      panic("pgfault(): sys_page_map failed");
  if (sys_page_unmap(0, PFTEMP) < 0) 
    panic("pgfault(): sys_page_unmap failed");	
  
  //panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	void *addr = (void *) (pn*PGSIZE);
  uint32_t perm = PTE_U | PTE_P;
  // Provjera za permisije, ako je stranica za pisanje ili copy-on-write
  if ((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW)) 
    perm |= PTE_COW;
  // Mapiramo na osnovu permisija koje smo dobili
  r = sys_page_map(0, addr, envid, addr, perm);
  if (r < 0) 
    panic("duppage(): sys_page_map failed");
  if (perm & PTE_COW) {
    r = sys_page_map(0, addr, 0, addr, perm);
    if (r < 0) 
      panic("duppage(): sys_page_map failed");
  }
  
  //panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
  set_pgfault_handler(pgfault);
  envid_t child = sys_exofork();
  // Ako se desi greska, panic
  if (child < 0) 
    panic("fork(): sys_exofork error");
  // Ako je child = 0, onda smo mi child, thisenv treba da pokazuje na child
  if (child == 0) {
    thisenv = &envs[ENVX(sys_getenvid())];
    return child;
  }
  // Ako smo uspjeli doci do ovog dijela, znaci da smo mi parent. Mapiramo
  uint32_t addr;
  for (addr = 0; addr < USTACKTOP; addr += PGSIZE) {
    if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_U)) 
      duppage(child, PGNUM(addr));
  }
  // Pravimo child user exception stack
  if (sys_page_alloc(child, (void *) (UXSTACKTOP-PGSIZE), PTE_P | PTE_U | PTE_W) < 0) 
    panic("fork(): sys_page_alloc failed to allocate a page for UE stack");
  // Postavljamo pgfault entry
  if (sys_env_set_pgfault_upcall(child, thisenv->env_pgfault_upcall) < 0)
    panic("fork(): sys_env_set_pgfault_upcall failed to set up entry for pgfault");
  // Postavljamo child kao runnable
  if (sys_env_set_status(child, ENV_RUNNABLE) < 0) 
    panic("fork(): sys_env_set_status failed to set child as runnable");
  return child;
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
