/* 
  filename: proc_listing.c

  Kernel module which finds all processes with > 3 files open.
  Compile with attached makefile.
  load with: sudo insmod proc_listing.ko
  Output is in /var/log/kern.log
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/unistd.h>
#include <asm/fcntl.h>
#include <asm/errno.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/tty.h>
#include <asm-generic/statfs.h>
#include "sys_addresses.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jennifer Weant");
MODULE_DESCRIPTION("This module lists the number of processes that have more than 3 files open when the module is loaded.  It prints it to the kern.log file.");

asmlinkage long (*sys_statfs)(const char __user *path,
  struct statfs __user *buf) = (long (*)(const char __user *path,
  struct statfs __user *buf))SYS_STATFS;

/*
  get_my_tasks

  For each process, go through its proc directory and extract 
   each file.  Use fstat to grab some information (type) and
   verify if the file exists.
*/
static int get_my_tasks()
{
  struct task_struct *p; // current task
  int number_of_procs = 0; // Number of structs w/ > 3 fds

  for_each_process(p)
  { 
    int fd_count = 0; // Number of fds for this process
    struct statfs buf; // Hold the file's status
    char my_file_name[25]; // filename: should never be greater than
                           //  /proc/32678/fd/255 in length
    long statfs_return_value = 0; // return value from statfs
    mm_segment_t oldfs; // Hold the memory segment when we
			// switch over to make sys_statfs work
    int j; // Loop counter for variables

    /* I found that count.counter was inconsistent on how many 
       files it was returning */ 
    //if (p->files->count.counter > 3)
    //  ++num;
 
    for (j = 3; j < 255; ++j) 
    {
      sprintf(my_file_name,"/proc/%d/fd/%d", p->pid, j);

      oldfs = get_fs(); 
      set_fs(KERNEL_DS); 
      statfs_return_value = sys_statfs(my_file_name, &buf);
      set_fs(oldfs);
      
      if (statfs_return_value >= 0)
      {
        int is_valid_file_type = 0;
         /* Just accept a smattering of filesystem types, not
            every single one available */
	switch (buf.f_type)
        {
        case 0xadf5: // active directory fs
        case 0xef53: // ext2, ext3, ext4
        case 0x6969: // NFS
        case 0x4d44: // MSDOS
        case 0x5346544e: // NTFS
        case 0x517b: // SMB
          is_valid_file_type = 1;
          break;
        default:
          // 0x9fa0: /proc files don't count
          is_valid_file_type = 0;
        }
        // My computer is only mapped onto ext filesystem
        if (is_valid_file_type)
        {
          ++fd_count;
        }
      }
    }

    if (fd_count > 3)
      ++number_of_procs;

//    printk(KERN_INFO " pid: %d %s %d %d", p->pid, 
//      get_task_comm(commtask, p),
//      p->files->count.counter,
//      fd_count);
  }

  printk(KERN_INFO "There are %d processes with more than 3 files open.", 
    number_of_procs);

  return 0;
} /* get_my_tasks */

/* 
 my_cleanup_module

 Doesn't do anything, I included it here for consistency.
*/ 
void my_cleanup_module(void) /*module shutdown*/ 
{ 
}

module_init(get_my_tasks)
module_exit(my_cleanup_module)
