#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/major.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
// #include <l4/sys/err.h>
// #include <l4/sys/types.h>
// #include <l4/re/env>
// #include <l4/re/util/cap_alloc>
//#include <l4/cxx/ipc_stream>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/namespace.h>
#include <l4/sys/ipc.h>



//#define MAJOR_NUM 200
//#define DEVICE_NAME "cdev"

int interval = 10;
int pwm_enable = 0;

static DEFINE_MUTEX(sysfs_lock);

static ssize_t onda_pincount_show(struct device *dev,
                struct device_attribute *attr, char *buf);
//static ssize_t onda_pincount_store(struct device *dev,
//                struct device_attribute *attr, const char *buf, size_t size);

static ssize_t onda_interval_show(struct device *dev,
                struct device_attribute *attr, char *buf);
static ssize_t onda_interval_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size);

static ssize_t onda_pwm_show(struct device *dev,
                struct device_attribute *attr, char *buf);
static ssize_t onda_pwm_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size);

static struct class_attribute onda_class_attr[] = {
	//__ATTR(pincount, 0644, onda_pincount_show, onda_pincount_store),
	__ATTR(button_count, 0644, onda_pincount_show, NULL),
	__ATTR(onda_interval, 0644, onda_interval_show, onda_interval_store),
	__ATTR(pwm_enable, 0644, onda_pwm_show, onda_pwm_store),
	__ATTR_NULL
};

static struct class onda_drv = {
	.name = "onda",
	.owner = THIS_MODULE,
	.class_attrs = &onda_class_attr,
};

static ssize_t onda_pincount_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	ssize_t status;
	l4_msgtag_t tag;
	l4_umword_t label;
	l4_cap_idx_t comm_cap;
	int pincount = 0;
	int ipc_error;

	mutex_lock(&sysfs_lock);
	
	//l4_utcb_mr()->mr[0] = 0;
	
	comm_cap = l4re_get_env_cap("comm");

	/* Get a free capability slot for the comm capability */
	if (l4_is_invalid_cap(comm_cap)) {
		printk("Did not find an comm\n");
		mutex_unlock(&sysfs_lock);
		return 0;
    }

	/* To an L4 IPC call, i.e. send a message to thread2 and wait for a
	* reply from thread2. The '1' in the msgtag denotes that we want to
	* transfer one word of our message registers (i.e. MR0). No timeout. */
	tag = l4_ipc_call(comm_cap, l4_utcb(), l4_msgtag(1, 0, 0, 0), L4_IPC_NEVER);
	
	/* Check for IPC error, if yes, print out the IPC error code, if not,
	* print the received result. */
	ipc_error = l4_ipc_error(tag, l4_utcb());
	if (ipc_error) {
		printk("IPC error: %x\n", ipc_error);
		return 0;
	} else {
		pincount = (int)(l4_utcb_mr()->mr[0]);
		printk("Received: %d\n", pincount);
	}

	status = sprintf(buf, "%d\n", pincount);
	
	mutex_unlock(&sysfs_lock);
	return status;
}

static ssize_t onda_interval_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	ssize_t status;

	mutex_lock(&sysfs_lock);
	
	status = sprintf(buf, "%d\n", interval);
	
	mutex_unlock(&sysfs_lock);
	return status;
}

static ssize_t onda_interval_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	if (sscanf(buf,"%d\n", &value) > 0) {
		interval = value;
	}
	return size;
}

static ssize_t onda_pwm_show(struct device *dev,
                struct device_attribute *attr, char *buf)
{
	ssize_t status;

	mutex_lock(&sysfs_lock);
	
	status = sprintf(buf, "%d\n", pwm_enable);
	
	mutex_unlock(&sysfs_lock);
	return status;
}

static ssize_t onda_pwm_store(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t size)
{
	int value;
	if (sscanf(buf,"%d\n", &value) > 0) {
		pwm_enable = value;
	}
	return size;
}

static int onda_init(void)
{
	int status;
	/*status = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
	if(status < 0)
	printk("Registering char driver failed with status = %d\n", status);
	else*/
	printk("Hello World2\n\n");
	
	status = class_register(&onda_drv);
	if(status < 0)
		printk("Registering Class Failed\n");

	return 0;
}

static void onda_exit(void)
{
	class_unregister(&onda_drv);
	//unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(" GoodBye, world\n");
}

module_init(onda_init);
//fs_initcall(onda_init);
module_exit(onda_exit);
MODULE_LICENSE("GPL");
