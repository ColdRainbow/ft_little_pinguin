// SPDX-License-Identifier: GPL-2.0-only

# include <linux/init.h>
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/fs.h>
# include <linux/mount.h>
# include <linux/nsproxy.h>
# include <linux/fs_struct.h>
# include <linux/sched.h>
# include <linux/dcache.h>
# include <linux/ns_common.h>
# include <linux/proc_fs.h>
# include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jamie Kate jkate@wolfsburg-student.de");
MODULE_DESCRIPTION("A kernel module to list mount points");

struct mount {
	struct hlist_node mnt_hash;
	struct mount *mnt_parent;
	struct dentry *mnt_mountpoint;
	struct vfsmount mnt;
	union {
		struct rcu_head mnt_rcu;
		struct llist_node mnt_llist;
	};
#ifdef CONFIG_SMP
	struct mnt_pcp __percpu *mnt_pcp;
#else
	int mnt_count;
	int mnt_writers;
#endif
	struct list_head mnt_mounts;
	struct list_head mnt_child;
	struct list_head mnt_instance;
	const char *mnt_devname;
	union {
		struct rb_node mnt_node;
		struct list_head mnt_list;
	};
};

struct mnt_namespace {
	struct ns_common ns;
	struct mount *root;
	struct rb_root mounts;
};


static int my_seq_show(struct seq_file *m, void *v)
{
	struct vfsmount *mnt;
	struct rb_root ns;
	struct mount *mnt_entry;
	struct mount *temp;
	struct path p;

	ns = current->nsproxy->mnt_ns->mounts;

	rbtree_postorder_for_each_entry_safe(mnt_entry, temp, &ns, mnt_node) {
		mnt = &mnt_entry->mnt;

		seq_printf(m, "%s	", mnt_entry->mnt_devname);

		p.mnt = mnt;
		p.dentry = mnt_entry->mnt.mnt_root;
		seq_path(m, &p, "");

		seq_printf(m, "\n");
	}


	return 0;
}

static int my_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, my_seq_show, NULL);
}

static const struct proc_ops my_proc_fops = {
	.proc_open    = my_seq_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
};

static int __init list_mounts_init(void)
{
	pr_info("Listing mount points:\n");

	proc_create("mymounts", 0, NULL, &my_proc_fops);

	return 0;
}

static void __exit list_mounts_exit(void)
{
	pr_info("Exiting list_mounts module\n");

	remove_proc_entry("mymounts", NULL);
}

module_init(list_mounts_init);
module_exit(list_mounts_exit);
