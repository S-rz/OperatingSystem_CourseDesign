#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/stat.h>

#define DEV_SIZE 102400 //设备申请的最大内存空间

struct Device
{
  char *data;//数组指针，用于存放从用户读入的数据
  long size;//存储的数据长度
}*devp;
struct cdev cdev;//字符设备结构体
static int devNum_major = 0;//主设备号变量

//对应用户态的lseek
static loff_t my_llseek(struct file *file,loff_t offset, int whence)
{
   	loff_t cfo=0;//文件当前偏移量，current file offset

    switch(whence){
    case 0://SEEK_SET
        cfo=offset;break;
  	case 1://SEEK_CUR
          cfo=file->f_pos+offset;break;
  	case 2://SEEK_END
          cfo=DEV_SIZE-1+offset;break;
    }
    if ((cfo<0) || (cfo>DEV_SIZE))//文件偏移量越界
     	return -EINVAL;
    file->f_pos = cfo;
    return cfo;
}

//对应用户态的read,写数据到用户空间
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *p_cfo)
{
  	int val = 0;
  	struct Device *dev = file->private_data; //设备描述结构体指针,获取设备信息

  	if (copy_to_user(buf, (void*)(dev->data + *p_cfo), count))//如果成功返回0；如果失败，返回未完成copy的长度
    	val = -EFAULT;
  	else
  	{
    	*p_cfo += count;//copy成功，文件偏移量加上count
    	val = count;
  	}

  	return val;
}

//对应用户态的write,从用户空间读入数据
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *p_cfo)
{
  	int val = 0;
  	struct Device *dev = file->private_data;

  	if (copy_from_user(dev->data + *p_cfo, buf, count))//如果成功返回0；如果失败，返回未完成copy的长度
    	val = -EFAULT;
  	else
  	{
    	*p_cfo += count;//copy成功，文件偏移量加上count
    	val = count;
  	}

  	return val;
}

static int my_open(struct inode *inode, struct file *file)
{
    file->private_data = devp;
    return 0;
}

struct file_operations fops={
  	.owner = THIS_MODULE,
  	.llseek = my_llseek,
  	.read = my_read,
  	.write = my_write,
  	.open = my_open,
};

int __init init_mymod(void)
{
	int dev_num;
	dev_num = register_chrdev(0,"MyDev",&fops);   //向系统的字符设备表登记一个字符设备
	if (dev_num < 0) { //返回负值表示注册失败
		printk(KERN_INFO "MyDev: FAIL to get major number\n");
		return dev_num;
	}
	if (devNum_major == 0) devNum_major = dev_num;

  	//初始化cdev结构
  	cdev_init(&cdev, &fops);
  	cdev.owner = THIS_MODULE;
  	cdev.ops = &fops;
  	cdev_add(&cdev,MKDEV(devNum_major, 0), 1);//注册1个字符设备

  	//为设备描述结构体分配内存
  	devp = kmalloc(sizeof(struct Device), GFP_KERNEL);
  	if (!devp)
  	{
    	dev_num = -ENOMEM;
    	printk(KERN_INFO "MyDev: FAIL to get memory\n");
    	return dev_num;
  	}
    (*devp).size = DEV_SIZE;
    (*devp).data = kmalloc(DEV_SIZE, GFP_KERNEL);
    memset((*devp).data, 0, (*devp).size);//初始化为0

  	return 0;
}

void __exit clean_mod(void)//注销
{
  	unregister_chrdev(devNum_major,"MyDev");//卸载字符设备
}

MODULE_LICENSE("GPL");
module_exit(clean_mod);
module_init(init_mymod);
