// SPDX-License-Identifier: GPL-2.0-only

/* Needed by all modules */
#include <linux/module.h>

/* Needed for pr_info() */
#include <linux/printk.h>

MODULE_LICENSE("GPL");

int init_module(void)
{
	pr_info("Hello world !\n");

	/* A non 0 return means init_module failed; module can't be loaded. */
	return 0;
}

void cleanup_module(void)
{
	pr_info("Cleaning up module.\n");
}
