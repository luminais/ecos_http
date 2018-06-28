/*****************************************************************************
Copyright (C), 吉祥腾达，保留所有版权
File name ：tenda_wlan_proc.c
Description : 无线驱动PROC文件，控制和查看tenda驱动私有添加的功能，如：band steering
Author ：dengxingde
Version ：V1.0
Date ：2017-5-4
Others ：
History ：修 改 历 史 记 录 列 表
1) 日期： 修改者：
  修改内容：
2）...
*****************************************************************************/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/seq_file.h>

#include "tenda_sta_steering.h"
#include "tenda_wlan_proc.h"
#include "tenda_dev_probe.h"

extern struct sta_steer g_steer;

static char *s_proc_tenda_name = "tenda_wlan";
static struct proc_dir_entry *s_proc_tenda_root = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)

static int sta_steer_read_proc(char *buf, char **start, off_t offset,
                                        int count, int *eof, void *data)
{
    int len = 0;

    //len += sprintf(buf+len,"rx_jiffs    hard_jiffs    cpu_jiffs\n");
    *eof = 1;
    
    return len;
}
static int sta_steer_write_proc(struct file *file, const char *buffer,
                                                    unsigned long count, void *data)
{

    return count;
}

/* 
 * proc file system add function
 */
static int 
tenda_wlan_addproc(char *funcname, read_proc_t *hookfuncr, write_proc_t *hookfuncw)
{
    struct proc_dir_entry *pe;
    
    if (!s_proc_tenda_root)
        return -1;
    
    if (hookfuncw == NULL) {
        pe = create_proc_read_entry(funcname, 0, s_proc_tenda_root, hookfuncr, NULL);
        if (!pe) {
            diag_printf("ERROR in creating read proc entry (%s)! \n", funcname);
            return -1;
        }
    } else {
        pe = create_proc_entry(funcname, S_IRUGO | S_IWUGO, s_proc_tenda_root);
        if (pe) {
            pe->read_proc = hookfuncr;
            pe->write_proc = hookfuncw;
        } else {
            printk("ERROR in creating proc entry (%s)! \n", funcname);
            return -1;
        }
    }
    
    return 0;
}
#else
#define TENDA_DECLARE_READ_PROC_FOPS(read_proc) \
	int read_proc##_open(struct inode *inode, struct file *file) \
	{ \
			return(single_open(file, read_proc, PDE_DATA(file_inode(file)))); \
	} \
	struct file_operations read_proc##_fops = { \
			.open			= read_proc##_open, \
			.read			= seq_read, \
			.llseek 		= seq_lseek, \
			.release		= single_release, \
	}

#define TENDA_DECLARE_WRITE_PROC_FOPS(write_proc) \
	static ssize_t write_proc##_write(struct file * file, const char __user * userbuf, \
		     size_t count, loff_t * off) \
	{ \
		return write_proc(file, userbuf,count, PDE_DATA(file_inode(file))); \
	} \
	struct file_operations write_proc##_fops = { \
			.write			= write_proc##_write, \
	}


#define TENDA_DECLARE_READ_WRITE_PROC_FOPS(read_proc,write_proc) \
	static ssize_t read_proc##_write(struct file * file, const char __user * userbuf, \
		     size_t count, loff_t * off) \
	{ \
		return write_proc(file, userbuf,count, PDE_DATA(file_inode(file))); \
	} \
	int read_proc##_open(struct inode *inode, struct file *file) \
	{ \
			return(single_open(file, read_proc, PDE_DATA(file_inode(file)))); \
	} \
	struct file_operations read_proc##_fops = { \
			.open			= read_proc##_open, \
			.read			= seq_read, \
			.write			= read_proc##_write, \
			.llseek 		= seq_lseek, \
			.release		= single_release, \
	}


#define TENDA_CREATE_PROC_READ_ENTRY(name, func, data) \
		proc_create_data(name, 0644, s_proc_tenda_root, &func##_fops, (void *)data)

#define TENDA_CREATE_PROC_READ_WRITE_ENTRY(name, func, write_func, data) \
		proc_create_data(name, 0644, s_proc_tenda_root, &func##_fops, (void *)data)

#define TENDA_CREATE_PROC_WRITE_ENTRY(name, write_func, data) \
		proc_create_data(name, 0644, s_proc_tenda_root, &write_func##_fops, (void *)data)


static int sta_steer_write_proc(struct file *file, const char *buffer,
                                                    unsigned long count, void *data)
{
    char tmp[32];

    if (count > sizeof(tmp))
    	return -EINVAL;

	if (buffer && !copy_from_user(tmp, buffer, count)) {
        tmp[count-1] = '\0';
        if (strncmp(tmp,"1",1) == 0) {
            g_steer.enable = 1;
        } else if (strncmp(tmp,"0",1) == 0) {
            g_steer.enable = 0;
        } else {
            char cmd[32];
            int val;

            sscanf(tmp,"%s %d",cmd,&val);
            if (strcmp(cmd,"sta_balance_enable") == 0) {
                g_steer.sta_balance_enable = val;
            } else if (strcmp(cmd,"rssi_lmt_2g") == 0) {
                g_steer.rssi_lmt_2g = val;
            } else if (strcmp(cmd,"rssi_lmt_5g") == 0) {
                g_steer.rssi_lmt_5g = val;
            } else if (strcmp(cmd,"auth_reject_limit") == 0) {
                g_steer.auth_reject_limit = val;
            } else if (strcmp(cmd,"auth_status") == 0) {
            	g_steer.auth_status = val;
            } else if (strcmp(cmd,"assoc_status") == 0) {
            	g_steer.assoc_status = val;
            } 		
        }
        //g_steer.enable = simple_strtoul(tmp, NULL, 0);
	}

    return count;
}

static int sta_steer_read_proc(struct seq_file *s, void *data)
{
    sta_steer_proc_show(s);

    return 0;
}

static int sta_debug_write_proc(struct file *file, const char *buffer,
                                                    unsigned long count, void *data)
{
    char tmp[32];
//    int val;

    if (count > sizeof(tmp))
    	return -EINVAL;

	if (buffer && !copy_from_user(tmp, buffer, count)) {
        tmp[count-1] = '\0';
        g_steer.debug = simple_strtoul(tmp, NULL, 0);
	}

    return count;
}


TENDA_DECLARE_READ_WRITE_PROC_FOPS(sta_steer_read_proc,sta_steer_write_proc);
TENDA_DECLARE_WRITE_PROC_FOPS(sta_debug_write_proc);


#endif
int tenda_wlan_proc_init(void)
{
    printk("TENDA WLAN: init proc \n");
    
    s_proc_tenda_root = proc_mkdir(s_proc_tenda_name, NULL);
    if (!s_proc_tenda_root) {
        printk("%s proc initialization failed! \n", s_proc_tenda_name);
        return -1;
    }

    //tenda_wlan_addproc("sta_steer",
    //    sta_steer_read_proc, sta_steer_write_proc);
    TENDA_CREATE_PROC_READ_WRITE_ENTRY("sta_steer", sta_steer_read_proc, sta_steer_write_proc, NULL);
    TENDA_CREATE_PROC_WRITE_ENTRY("sta_debug", sta_debug_write_proc, NULL);
    return 0;
}

void tenda_wlan_proc_exit(void)
{
    printk("TENDA WLAN: remove proc \n");
    /* remove proc entries */

    remove_proc_entry("sta_steer", s_proc_tenda_root);
    remove_proc_entry("sta_debug", s_proc_tenda_root);
    remove_proc_entry(s_proc_tenda_name,NULL);
}
