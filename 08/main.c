// SPDX-License-Identifier: GPL-2.0-only

# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/init.h>
# include <linux/miscdevice.h>
# include <linux/fs.h>
# include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jamie Kate <jkate@student.42wolfsburg.de>");
MODULE_DESCRIPTION("Module that creates a miscellaneous character device");

static ssize_t myfd_read(struct file *fp,
			 char __user *user,
			 size_t size,
			 loff_t *offs
);

static ssize_t myfd_write(struct file *fp, const char __user *user,
			  size_t size, loff_t *offs);

static const struct file_operations myfd_fops = {
	.owner = THIS_MODULE,
	.read = &myfd_read,
	.write = &myfd_write
};

static struct miscdevice myfd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &myfd_fops
};

char str[PAGE_SIZE]; char *tmp;

static int __init myfd_init(void)
{
	int retval;

	retval = misc_register(&myfd_device);
	if (retval) {
		pr_err("Failed to register misc device\n");
		return retval;
	}
	pr_info("Misc device registered\n");
	return 0;
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
	pr_info("Misc device unregistered\n");
}

ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs)
{
	size_t t, i;
	char *tmp2;

	if (str[PAGE_SIZE - 1] != '\0')
		str[PAGE_SIZE - 1] = '\0';

	/*
	 *Malloc like a boss
	 */
	tmp2 = kmalloc(sizeof(char) * PAGE_SIZE * 2, GFP_KERNEL);

	if (!tmp2)
		return -ENOMEM;

	tmp = tmp2;

	for (t = strlen(str) - 1, i = 0; t < SIZE_MAX && i < PAGE_SIZE; t--,
	     i++)
		tmp[i] = str[t];
	tmp[i] = '\0';

	ssize_t res = simple_read_from_buffer(user, size, offs, tmp, i);

	kfree(tmp2);

	return res;
}

ssize_t myfd_write(struct file *fp, const char __user *user,
		   size_t size, loff_t *offs)
{
	ssize_t res;

	if (size > PAGE_SIZE - 1)
		size = PAGE_SIZE - 1;

	res = simple_write_to_buffer(str, size, offs, user, size);

	if (res >= 0)
		str[res] = '\0';
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup);
