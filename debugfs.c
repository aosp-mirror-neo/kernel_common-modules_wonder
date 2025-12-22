// SPDX-License-Identifier: GPL-2.0
/*
 * Google Wonder WiFi Virtual Soft-MAC Driver
 *
 * Debugfs implementation for the Wonder driver.
 */
#define pr_fmt(fmt) "[wonder][debugfs] " fmt
#define LOG_MODULE_NAME "debugfs"

#include <linux/debugfs.h>
#include <linux/netdevice.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>

#include "core.h"
#include "mac80211.h"
#include "wondertap_internal.h"

static int wonder_version_show(struct seq_file *m, void *v)
{
	struct wonder_data *wonder = m->private;

	seq_printf(m, "%u\n", wonder->wondertap_data.ver);
	return 0;

}
DEFINE_SHOW_ATTRIBUTE(wonder_version);

static int wonder_capabilities_show(struct seq_file *m, void *v)
{
	struct wonder_data *wonder = m->private;
	struct wondertap_capability caps;
	int ret;

	if (!wonder) {
		pr_err("wondertap not available\n");
		return -ENODEV;
	}

	ret = wondertap_get_capabilities(&wonder->wondertap_data, &caps);
	if (ret) {
		pr_err("Failed to get wondertap capabilities, error: %d\n", ret);
		return -EOPNOTSUPP;
	}

	seq_printf(m, "%08x\n", caps.raw_bits);
	return 0;
}
DEFINE_SHOW_ATTRIBUTE(wonder_capabilities);

static ssize_t wonder_force_stop_tx_write(struct file *file,
					  const char __user *user_buf,
					  size_t count, loff_t *ppos)
{
	struct wonder_data *wonder = file->private_data;
	bool stop;
	int ret;

	if (!wonder || !wonder->vdev) {
		pr_err("vdev not available\n");
		return -ENODEV;
	}

	ret = kstrtobool_from_user(user_buf, count, &stop);
	if (ret)
		return ret;

	wonder->tx_stop = stop;

	return count;
}

static const struct file_operations fops_force_stop_tx = {
	.owner = THIS_MODULE,
	.write = wonder_force_stop_tx_write,
	.open = simple_open,
	.llseek = noop_llseek,
};

void wonder_debugfs_init(void *wonder)
{
	struct dentry *wonder_debugfs_root;

	wonder_debugfs_root = debugfs_create_dir("wonder", NULL);
	debugfs_create_file("force_stop_tx", 0200, wonder_debugfs_root,
			    wonder, &fops_force_stop_tx);
	debugfs_create_file("version", 0400, wonder_debugfs_root, wonder, &wonder_version_fops);
	debugfs_create_file("capabilities", 0400, wonder_debugfs_root, wonder,
			    &wonder_capabilities_fops);
}

void wonder_debugfs_exit(void)
{
	debugfs_lookup_and_remove("wonder", NULL);
}
