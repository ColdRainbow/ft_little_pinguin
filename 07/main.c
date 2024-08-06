// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>
/*Needed by all modules*/

#include <linux/printk.h>
/*Needed for pr_info()*/

#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jkate jkate@wolfsburg-student.de");
MODULE_DESCRIPTION("A simple example to create a file \
		in the fortytwo directory in debugfs \
		that is writable only by root \
		and readable by anyone");

static struct dentry *fortytwo_dir;

int major;
char str[PAGE_SIZE];

// Mutex for locking access to "foo"
static struct mutex foo_mutex;

// Buffer to store data for "foo"
static char foo_value[PAGE_SIZE];

// Maximum size for "foo" (one page)
static size_t foo_size = PAGE_SIZE;

static ssize_t id_file_read(struct file *filp,
		char __user *buffer,
		size_t length,
		loff_t *offset)
{
	return simple_read_from_buffer(buffer, length, offset, "jkate\n", 6);
}

static ssize_t id_file_write(struct file *filp,
		const char __user *buffer,
		size_t length,
		loff_t *offset)
{
	int num = simple_write_to_buffer(str, length, offset, buffer, length);

	if (num < 0)
		return -1;
	str[num] = '\0';

	int res = strcmp(str, "jkate");

	if (res == 0)
		return num;
	return -EINVAL;
}

static ssize_t jiffies_file_read(struct file *file,
		char __user *buf_in_user, size_t count, loff_t *pos)
{
	char buffer[20];
	int len;

	if (*pos > 0)
		return 0;

	unsigned long current_jiffies = jiffies;

	len = snprintf(buffer, sizeof(buffer), "%lu\n", current_jiffies);

	if (copy_to_user(buf_in_user, buffer, len) != 0) {
		// Error copying data to user space
		return -EFAULT;
	}

	// I can get this offset from user space
	*pos += len;

	return len;
}

static ssize_t foo_file_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	int res = 0;

	mutex_lock(&foo_mutex);

	if (*pos >= foo_size) {
		// EOF
		res = 0;
	} else {
		if (*pos + count > foo_size)
			count = foo_size - *pos;
		if (copy_to_user(buf, foo_value + *pos, count)) {
			// Error copying data to user space
			res = -EFAULT;
		} else {
			*pos += count;
			res = count;
		}
	}

	mutex_unlock(&foo_mutex);

	return res;
}

static ssize_t foo_file_write(struct file *file,
		const char __user *buf, size_t count, loff_t *pos)
{
	ssize_t res = 0;

	mutex_lock(&foo_mutex);

	if (*pos >= foo_size) {
		res = -EINVAL;
		goto out;
	}

	size_t remaining_size = foo_size - *pos;

	if (count > remaining_size)
		count = remaining_size;

	if (copy_from_user(foo_value + *pos, buf, count)) {
		// Error copying data from user space
		res = -EFAULT;
		goto out;
	}

	*pos += count;
	res = count;

out:
	mutex_unlock(&foo_mutex);
	return res;
}

// Fops for id
static const struct file_operations id_file_fops = {
	.owner = THIS_MODULE,
	.read = id_file_read,
	.write = id_file_write,
};

// Fops for jiffies
static const struct file_operations jiffies_file_fops = {
	.owner = THIS_MODULE,
	.read = jiffies_file_read,
};

// Fops for foo
static const struct file_operations foo_file_fops = {
	.owner = THIS_MODULE,
	.read = foo_file_read,
	.write = foo_file_write,
};

int __init init_module(void)
{
	pr_info("Hello world !\n");

	fortytwo_dir = debugfs_create_dir("fortytwo", NULL);
	if (!fortytwo_dir) {
		pr_err("Cannot create the directory");
		return -ENOMEM;
	}

	debugfs_create_file("id", 0666, fortytwo_dir, NULL, &id_file_fops);
	debugfs_create_file("jiffies", 0444, fortytwo_dir,
			NULL, &jiffies_file_fops);
	debugfs_create_file("foo", 0644, fortytwo_dir, NULL, &foo_file_fops);

	// A non-0 return means init_module failed; module can't be loaded.
	return 0;
}

void __exit cleanup_module(void)
{
	pr_info("Cleaning up module.\n");

	debugfs_remove_recursive(fortytwo_dir);
}
