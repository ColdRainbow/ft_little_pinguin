// SPDX-License-Identifier: GPL-2.0-only

/* Needed by all modules */
# include <linux/module.h>

/* Needed for pr_info() */
#include <linux/printk.h>

# include <linux/module.h>
# include <linux/init.h>
# include <linux/miscdevice.h>
# include <linux/fs.h>
# include <linux/uaccess.h>

MODULE_LICENSE("GPL");

int major;
char str[PAGE_SIZE];
static struct class *cls;

static ssize_t device_read(struct file *filp,
		char __user *buffer,
		size_t length,
		loff_t *offset)
{
	return simple_read_from_buffer(buffer, length, offset, "jkate\n", 6);
}

static ssize_t device_write(struct file *filp,
		const char __user *buffer,
		size_t length,
		loff_t *offset)
{
	int num = simple_write_to_buffer(str, length, offset, buffer, length);

	if (num < 0)
		return -1;
	str[num] = '\0';

	int res = strcmp(str, "jkate");

	if (res == 0) {
		pr_err("Invalid argument.\n");
		return num;
	}
	return -EINVAL;
}

const struct file_operations fops = {
read: device_read,
write : device_write,
};

int init_module(void)
{
	pr_info("Hello world !\n");
	major = register_chrdev(0, "test", &fops);
	cls = class_create("fortytwo");
	device_create(cls, NULL, MKDEV(major, 0), NULL, "fortytwo");

	/* A non-0 return means init_module failed; module can't be loaded. */
	return 0;
}

void cleanup_module(void)
{
	pr_info("Cleaning up module.\n");
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "test");
}
