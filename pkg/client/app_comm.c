//static /* const */ DEVICE_ATTR(pincount, 0644,
//                onda_pincount_show, onda_pincount_store);

/*static struct class gpio_class = {
        .name =         "onda",
        .owner =        THIS_MODULE,

        .class_attrs =  gpio_class_attrs,
};*/

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


//#define MAJOR_NUM 200
//#define DEVICE_NAME "cdev"

int pincount = 100;
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

	mutex_lock(&sysfs_lock);
	
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
	printk("Hello World\n");

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
module_exit(onda_exit);
MODULE_LICENSE("GPL");