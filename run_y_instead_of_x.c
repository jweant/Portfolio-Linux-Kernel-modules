/*
  run_y_instead_of_x.c

  Anytime anyone tries to run x, y is run instead.
*/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/unistd.h>
#include <linux/syscalls.h>
#include <asm/amd_nb.h>
#include <linux/highuid.h>
#include <linux/namei.h>
#include <linux/tty.h>
#include "sys_addresses.h"

#define GPF_DISABLE write_cr0(read_cr0() & (~ 0x10000))
#define GPF_ENABLE write_cr0(read_cr0() | 0x10000)

MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Jennifer Weant");
MODULE_DESCRIPTION("Anytime someone tries to run program x, y is run instead.  Please don't set y to something nasty.");

static char *y = DEFAULT_Y;
module_param(y, charp, 0000);
MODULE_PARM_DESC(y, "y: replacement executable to run instead of x");

static char *x = DEFAULT_X;
module_param(x, charp, 0000);
MODULE_PARM_DESC(x, "x: executable to that y replaces");

void **sys_call_table = (void **)SYS_CALL_TABLE;

int flag = 0;

/* 
These calls to execve will need to be modified for different processors.
The prototypes for this differ based on architecture.
*/
asmlinkage int (*original_sys_execve) (const char __user * ufilename, 
const char __user *const __user *uargv,
const char __user *const __user *uenvp,
struct pt_regs __regs); 

asmlinkage int fake_execve_function(
const char __user *ufilename,
const char __user *const __user *uargv,
const char __user *const __user *uenvp,
struct pt_regs __regs)
{
  int error;
  struct path nd, nd_t;
  struct inode *inode,*inode_t;
  mm_segment_t fs;
 
  fs = get_fs();
  set_fs(get_ds());
 
  error = user_path(ufilename, &nd);

  set_fs(fs);
//  printk(KERN_INFO "ufilename: %s", ufilename);        

  if(!error)
  {
    inode=nd.dentry->d_inode;

    /* Allow kernel space to use user space */
    fs=get_fs( );
    set_fs(get_ds( ));

    error = user_path(x, &nd_t);

    set_fs(fs);
    printk(KERN_INFO "x: %s y: %s current: %s", x, y, ufilename);

    if(!error)
    {
      inode_t=nd_t.dentry->d_inode;

      if(inode==inode_t)
      {
        int ret = 0;
        mm_segment_t old_fs = get_fs();
	printk(KERN_INFO "%s is NOT allowed: try %s", x, y);
        set_fs(KERNEL_DS);		
        ret = original_sys_execve(y, uargv, uenvp, __regs);
        set_fs(old_fs);
        return ret;
      }
    }
  }
  
  return original_sys_execve(ufilename, uargv, uenvp, __regs);
} /* fake_execve_function */

/* 
  set_addr_rw       

  Set RW permissions 
*/
void set_addr_rw(unsigned long addr) 
{
  unsigned int level;
  pte_t *pte = lookup_address(addr, &level);
  if (pte->pte &~ _PAGE_RW) pte->pte |= _PAGE_RW;
} /* set_addr_rw */

/* 
  set_addr_ro

  Set RO permissions at address
*/
void set_addr_ro(unsigned long addr) 
{
  unsigned int level;
  pte_t *pte=lookup_address(addr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
} /* set_addr_ro */

/*
  my_init

  Initialization, exchanges the system execve function with our execve function.
*/ 
static int __init my_init (void)
{
  flag = 1;
  if (flag)
  {
    printk(KERN_INFO "Exchanging %s for %s",x,y);
    set_addr_rw((unsigned long)sys_call_table);
    GPF_DISABLE;
    original_sys_execve =(void * )xchg(&sys_call_table[__NR_execve],
    fake_execve_function);
  }                                
  return 0;

} /* my_init */
       
/* 
  my_exit

  Remove patch from sys table
*/ 
static void my_exit (void)
{
  printk(KERN_INFO "Removing intercept_execve, flag: %d", flag);
  if (flag)
  {
    xchg(&sys_call_table[__NR_execve], original_sys_execve);
    set_addr_ro((unsigned long)sys_call_table);
    GPF_ENABLE;
    flag=0;
  }
} /* my_exit */
        
module_init(my_init);
module_exit(my_exit);

