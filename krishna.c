/* This module is created to write in file current time
*/

#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lebedev Timofey");
MODULE_DESCRIPTION("Time logging module");
MODULE_VERSION("0.0.1.3");

// Global variables, 2 for timer, 2 for main function
int unload_flag;

// static struct hrtimer htimer;
// static ktime_t kt_periode;

char *buffer; 					//char buffer
struct file *time_file;			//opened file

//drivers for file work

struct file *driver_file_open(const char *path, int flags, int mode)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open("/opt/module/time.txt", O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
    set_fs(oldfs);
    return (filp);
}

void driver_file_close(struct file *filp)
{
    filp_close(filp, NULL);
}

int driver_file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    int ret;
    mm_segment_t oldfs;
    loff_t pos = offset;
    oldfs = get_fs();
    set_fs(get_ds());
	
    vfs_setpos(file, pos, pos + PAGE_SIZE);
    spin_lock(&file->f_lock);
    file->f_pos = pos;
    file->f_version = 0;
    spin_unlock(&file->f_lock);
    ret = kernel_write(file, data, size, pos);
    //vfs_fsync(file, 0);
    set_fs(oldfs);
    return (ret);
}

// Get time function

void time_to_string(unsigned char *buffer){
	unsigned long get_time;
    int sec, hr, min, tmp1,tmp2;
    struct timeval tv;
	char time_str[5] = "hh:mm";
	
    do_gettimeofday(&tv);
    get_time = tv.tv_sec;
    sec = get_time % 60;
    tmp1 = get_time / 60;
    min = tmp1 % 60;
    tmp2 = tmp1 / 60;
    hr = (tmp2 % 24) + 3; // [3,27] TODO:

	time_str[0] = (hr / 10) + '0';
	time_str[1] = (hr % 10) + '0';
	time_str[3] = (min / 10) + '0';
	time_str[4] = (min % 10) + '0';
	strcpy(buffer, time_str);
	
	return;
}

static void driver_file_opw(void){ //Open, write, close file
	time_file = driver_file_open("/root/module/time.txt", O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
	time_to_string(buffer);
	driver_file_write(time_file, 0, buffer, 5);
	driver_file_close(time_file);
}


// Timer functions

static void loop_timer (void){
	while (unload_flag == 0){
		driver_file_opw();
		msleep(6000);
	}
	return;
}

// Main function

static int __init time_module_init(void) {
	char time_str[5] = "TESTT";
	unload_flag = 0;
	//time_file = driver_file_open("/root/module/time.txt", O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
	buffer = kmalloc(5, GFP_HIGHUSER);
	strcpy(buffer, time_str);
	
	if(IS_ERR(time_file)){
		// if file opened incorrectly
		printk(KERN_INFO "Time logging module: loaded unsuccesfully\n");
		kfree(buffer);
		return -EIO;
	}
	else{
		//time_to_string(buffer);
		//driver_file_write(time_file, 0, buffer, 5);
		//driver_file_opw();
		loop_timer();
	}
	return 0;
}

// exit function

static void __exit time_module_exit(void) {
	//timer_cleanup();
	//driver_file_close(time_file);
	unload_flag = -1;
	msleep(6000);
	kfree(buffer);
	printk(KERN_INFO "Time logging module: unloaded succesfully\n");
}

module_init(time_module_init);
module_exit(time_module_exit);
