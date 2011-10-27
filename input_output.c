/* 
 input_output.c

 Reads in a string from stdin and prints it out to stdout (or console).
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>	/* For current */
#include <linux/tty.h>		/* For the tty declarations */
#include <linux/version.h>	/* For LINUX_VERSION_CODE */
#include <linux/file.h>
#include "sys_addresses.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jennifer Weant");
MODULE_DESCRIPTION("Upon installation this module prompts the user for input then repeats the input.");

asmlinkage long (*sys_read)(unsigned int fd, char __user *buf, size_t count)=
  (long (*)(unsigned int, char __user *, size_t))SYS_READ;

/*
  scan_string

  Reads string from tty and saves it
*/
static int scan_string(char *str, int max)
{
  int len = 0;
  struct tty_struct *my_tty;
  my_tty = current->signal->tty;
  if (my_tty != NULL)
  {
	mm_segment_t oldfs;
	loff_t off = 0;
	struct file *f = fget(0);
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	len = f->f_op->read(f, str, max, &off);
	set_fs(oldfs); 
  }
  return len;
} /* scan_string */
/*
 print_string

 outputs string to tty
*/ 
static void print_string(char *str)
{
	struct tty_struct *my_tty;

	/* 
	 * tty struct went into signal struct in 2.6.6 
	 */
	/* 
	 * The tty for the current task, for 2.6.6+ kernels 
	 */
	my_tty = current->signal->tty;

	/* 
	 * If my_tty is NULL, the current task has no tty you can print to 
	 * (ie, if it's a daemon).  If so, there's nothing we can do.
	 */
	if (my_tty != NULL) 
        {

		((my_tty->driver)->ops->write) (my_tty,	/* The tty itself */
					   str,	/* String                 */
					   strlen(str));	/* Length */

		((my_tty->driver)->ops->write) (my_tty, "\015\012", 2);
	}
} /* print_string */
/*
  print_string_init

  Runs at init.  Prompts user to enter input then repeats it.  
  init then exits.
*/
static int __init print_string_init(void)
{
	char buffer[80];
        int size=0;
	char *ptr = buffer;
	int i = 80;
	while (i > 0)
 	{
		*ptr = 0;
		++ptr;
		--i;
	}
	print_string("The module has been inserted. ");
        print_string("Enter your input: ");
        size=scan_string(buffer,80);
	printk(KERN_INFO "%d: %s", size, buffer);
	print_string("Was this your input? ");
        print_string(buffer);
	return 0;
} /* print_string_init */

/* 
 print_string_exit
 Tell the user the module is being removed
*/
static void __exit print_string_exit(void)
{
	printk(KERN_INFO "Removing module");
	print_string("The module has been removed.  Farewell world!");
}

module_init(print_string_init);
module_exit(print_string_exit);

