#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include "charDeviceDriver.h"

MODULE_LICENSE("GPL");

DEFINE_MUTEX(rw_lock);

// Linked list implementation
// --------------------------
int ls_append(struct linked_list *ls, char *buf, size_t len)
{
	struct node *cur = ls->head;
	if (!cur) {
		ls->head = kmalloc(sizeof(struct node), GFP_KERNEL);
		if (!ls->head)
			return 0;

		ls->head->msg = kmalloc(sizeof(char) * len, GFP_KERNEL);
		strcpy(ls->head->msg, buf);
		ls->head->next = NULL;
	} else {
		while (cur->next)
			cur = cur->next;

		cur->next = kmalloc(sizeof(struct node), GFP_KERNEL);
		if (!cur->next)
			return 0;
		cur->next->msg = kmalloc(sizeof(char) * len, GFP_KERNEL);
		strcpy(cur->next->msg, buf);
		cur->next->next = NULL;
	}

	// Update total size
	ls->total_sz += len * sizeof(char);

	return 1;
}

int ls_remove(struct linked_list *ls, char **buf)
{
	struct node *next_node = NULL;

	if (ls->head == NULL)
		return 0;

	next_node = ls->head->next;

	int len = strlen(ls->head->msg) + 1;
	size_t msg_sz = len * sizeof(char);

	*buf = kmalloc(msg_sz, GFP_KERNEL);
	strcpy(*buf, ls->head->msg);

	// Update total size
	ls->total_sz -= msg_sz;

	kfree(ls->head->msg);
	kfree(ls->head);
	ls->head = next_node;

	return 1;
}

int ls_destroy(struct linked_list *ls)
{
	struct node *cur = ls->head;
	struct node* tmp;

	while (cur != NULL) {
		tmp = cur;
		cur = cur->next;
		kfree(tmp);
	}

	ls->head = NULL;
	ls->total_sz = 0;

	return 1;
}

// Character device driver implementation
// --------------------------------------

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int new_max = (int)ioctl_param;

	if (new_max > max_msg_ls_len || new_max > msg_ls.total_sz) {
		printk("\n\nNew max: %d\n\n", new_max);
		max_msg_ls_len = new_max;
		return SUCCESS;
	}

	printk("Ma nooooo\n");

	return -EINVAL;
}

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	msg_ls.head = NULL;
	msg_ls.total_sz = 0;

	// DO NOT CHANGE THESE LINES HERE, THEY ARE USED BY THE TEST SCRIPTS TO
	// FIND THE RIGHT MAJOR/MINOR NUMBERS
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

	return SUCCESS;
}

void cleanup_module(void)
{
	// Unregister the device and clean linked list
	unregister_chrdev(Major, DEVICE_NAME);
	ls_destroy(&msg_ls);
}

static int device_open(struct inode *inode, struct file *file)
{
	/*mutex_lock(&dev_lock);
	if (Device_Open) {
		mutex_unlock(&dev_lock);
		return -EBUSY;
	}
	Device_Open++;
	mutex_unlock(&dev_lock);*/

	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	/*mutex_lock(&dev_lock);
	Device_Open--;
	mutex_unlock(&dev_lock);*/
	module_put(THIS_MODULE);

	return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
	int bytes_read = 0;

	mutex_lock(&rw_lock);
	if (!ls_remove(&msg_ls, &msg_ptr)) {
		mutex_unlock(&rw_lock);
		return -EAGAIN;
	}

	printk("Read string: %s\n", msg_ptr);
	printk("MAX message size: %d\n", max_msg_len);
	printk("List message size: %d\n", msg_ls.total_sz);
	printk("MAX messages size: %d\n\n", max_msg_ls_len);

	if (*msg_ptr == 0) {
		mutex_unlock(&rw_lock);
		return 0;
	}

	char *ptr = msg_ptr;

	while (length && *ptr) {
		put_user(*(ptr++), buffer++);

		length--;
		bytes_read++;
	}

	if (*ptr == '\0')
		put_user(*ptr, buffer);

	kfree(msg_ptr);
	msg_ptr = NULL;
	mutex_unlock(&rw_lock);

	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
	int len_nb = length + 1;
	size_t msg_sz = len_nb * sizeof(char);

	printk("Message size: %d\n", msg_sz);
	printk("MAX message size: %d\n", max_msg_len);
	printk("List message size: %d\n", msg_ls.total_sz);
	printk("MAX messages size: %d\n\n", max_msg_ls_len);

	if (msg_sz > max_msg_len) {
		printk(KERN_ALERT "Error: the message size is bigger than 4K");
		return -EINVAL;
	}

	if (msg_ls.total_sz + msg_sz > max_msg_ls_len) {
		printk(KERN_ALERT "Error: the total messages size is bigger than the maximum allowed");
		return -EAGAIN;
	}

	// Get string from user space
	mutex_lock(&rw_lock);
	char *buf = kmalloc(sizeof(char) * len_nb, GFP_KERNEL);
	strncpy_from_user(buf, buffer, len_nb - 1);
	buf[len_nb - 1] = '\0';

	if (!ls_append(&msg_ls, buf, len_nb))
		printk(KERN_ALERT "An error occurred when adding the message");

	kfree(buf);
	mutex_unlock(&rw_lock);

	return length;
}
