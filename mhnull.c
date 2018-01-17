/*
 mhnull Copyright (C) <2016>  <matthias holl>
 kernel device driver like /dev/null
 logical device /dev/mhnull

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
	#include <linux/module.h>
	#include <linux/fs.h>
	#include <linux/device.h>
	#include <linux/cdev.h>
	#include <linux/uaccess.h>
	#define WRITE_POSSIBLE (atomic_read(&mahoo) != 0)
	static dev_t mhnull_dev_number;
	static struct cdev *driver_object;
	static struct class *mhnull_class;
	static struct device *mhnull_dev;
	static int driver_open(struct inode *device, struct file *instanz)
	{
		dev_info(mhnull_dev, "mhnull driver loaded\n");
		return 0;
	}
	static int driver_close (struct inode *device, struct file *instanz)
	{
		dev_info(mhnull_dev, "mhnull driver closed\n");
		return 0;
	}
	static ssize_t driver_write(struct file *instanz, const char __user *user, size_t count, loff_t *offset)
	{
		*offset += count;
		return count;
	}
	static struct file_operations fops = {
			.owner = THIS_MODULE,
			.open = driver_open,
			.write  = driver_write,
			.release = driver_close,
	};
	static int __init mod_init(void)
	{
		if (alloc_chrdev_region(&mhnull_dev_number, 0, 1, "mhnull") < 0)
			return -EIO;
		driver_object = cdev_alloc();
	if (driver_object == NULL)
		goto free_device_number;
	driver_object->owner = THIS_MODULE;
	driver_object->ops = &fops;
	if (cdev_add(driver_object, mhnull_dev_number, 1))
		goto free_cdev;
	mhnull_class = class_create(THIS_MODULE, "mhnull");
	if (IS_ERR(mhnull_class)) {
		pr_err("mhnull: no udev support\n");
		goto free_cdev;
	}
	mhnull_dev = device_create(mhnull_class, NULL, mhnull_dev_number, NULL, "%s", "mhnull");
	if (IS_ERR(mhnull_dev)) {
		pr_err("mhnull:device create failed\n");
		goto free_class;
	}
	return 0;
	free_class:
		class_destroy(mhnull_class);
	free_cdev:
		kobject_put(&driver_object->kobj);
	free_device_number:
		unregister_chrdev_region(mhnull_dev_number, 1);
		return -EIO;
		}
	static void __exit mod_exit(void)
	{
	device_destroy(mhnull_class, mhnull_dev_number);
	class_destroy(mhnull_class);
	cdev_del(driver_object);
	unregister_chrdev_region(mhnull_dev_number, 1);
	return;
	}
	module_init(mod_init);
	module_exit(mod_exit);
	MODULE_AUTHOR("matthias holl");
	MODULE_LICENSE("GPL");