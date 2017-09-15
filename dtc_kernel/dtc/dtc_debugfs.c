#include "dtc_debugfs.h"

/* main directory */
static struct dentry *main_dir;

/* enable */
static struct dentry *file_enable;
u32 dtc_debugfs_enable = 0;

/* timestamp */
static struct dentry *file_time_loc;
u32 dtc_debugfs_time_loc = 0;
static struct timeval timestamp;

/* target */
static struct dentry *file_target;
static char target_ip_port[24] = "1.1.1.1 1\n";
u32 dtc_debugfs_target_ip = 0;
u16 dtc_debugfs_target_port = 0;

/* info */
static struct dentry *file_info;
#define INFO_BUF_SIZE   33
static u8 info_buf[INFO_BUF_SIZE];

/* log1 */
static struct dentry *file_log1;
static const u64 log1_buf_size = 100 * 1024 * 1024;
static u64 log1_buf_pos = 0;
static struct debugfs_blob_wrapper log1_blob;

/* -------- implementation -------- */

/* ---- info ---- */
static ssize_t info_read_file(struct file *file, char __user *user_buf,
        size_t count, loff_t *ppos){
    int buf_len = snprintf(info_buf, INFO_BUF_SIZE,
            "HZ: %-4d\n"
            "int: %-1d\n"
            "char: %-1d\n"
            "long: %-1d\n"
            , HZ, (int)sizeof(int), (int)sizeof(char), (int)sizeof(long));
    return simple_read_from_buffer(user_buf, count, ppos, info_buf, buf_len);
}
static struct file_operations info_fops = {
    .read = info_read_file,
};

/* ---- target ---- */
static ssize_t target_read_file(struct file *file, char __user *user_buf,
        size_t count, loff_t *ppos){
    return simple_read_from_buffer(user_buf, count, ppos, 
            target_ip_port, strlen(target_ip_port));
}
static ssize_t target_write_file(struct file *file, const char __user *user_buf,
        size_t count, loff_t *ppos){
    int i = 0;
    int substr_index = 0;
    int sub_ip = 0;
    u32 ip_temp = 0;
    u16 port_temp = 0;
    
    if (count >= sizeof(target_ip_port)) // length error
        return -EINVAL;
    if (simple_write_to_buffer(target_ip_port, sizeof(target_ip_port), 
                ppos, user_buf, count) != count)
        return -EINVAL;

    for (i = 0; i < count; i++){
        if (target_ip_port[i] >= '0' && target_ip_port[i] <= '9'){ // number
            if (substr_index == 0){
                sub_ip = 10*sub_ip + (target_ip_port[i]-'0');
            } else if (substr_index == 1){
                port_temp = 10*port_temp + (target_ip_port[i]-'0');
            }
        } else if (target_ip_port[i] == '.'){ // delimiter in ip
            ip_temp = (ip_temp << 8) + sub_ip;
            sub_ip = 0;
        } else if (target_ip_port[i] == ' '){ // delimiter between ip and port
            ip_temp = (ip_temp << 8) + sub_ip;
            sub_ip = 0;
            substr_index++;
        }
    }
    
    dtc_debugfs_target_ip = htonl(ip_temp);
    dtc_debugfs_target_port = htons(port_temp);
    
    /* rewind to buffer beginning */
    log1_buf_pos = 0;
    log1_blob.size = 0;
    
    return count;
}
static struct file_operations target_fops = {
    .read = target_read_file,
    .write = target_write_file,
};




/* initialization */
int dtc_init_debugfs(char *dirname){
    /* main directory */
    main_dir = debugfs_create_dir(dirname, 0);
    if (!main_dir){
        printk(KERN_ALERT "dtc: main dir failed!\n");
        return -1;
    }
    /* enable */
    file_enable = debugfs_create_u32("enable", 0666, main_dir, 
            &dtc_debugfs_enable);
    if (!file_enable){
        printk(KERN_ALERT "dtc: enable file failed!\n");
        return -1;
    }
    /* timestamp */
    file_time_loc = debugfs_create_u32("timeLoc", 0666, main_dir,
            &dtc_debugfs_time_loc);
    if (!file_time_loc){
        printk(KERN_ALERT "dtc: timeLoc file failed!\n");
        return -1;
    }
    /* target */
    file_target = debugfs_create_file("target", 0666, main_dir, NULL,
            &target_fops);
    if (!file_target){
        printk(KERN_ALERT "dtc: target file failed!\n");
        return -1;
    }
    /* info */
    file_info = debugfs_create_file("info", 04444, main_dir, NULL,
            &info_fops);
    if (!file_info){
        printk(KERN_ALERT "dtc: info file failed!\n");
        return -1;
    }
    /* log1 */
    log1_blob.data = vmalloc(log1_buf_size);
    log1_blob.size = 0;
    file_log1 = debugfs_create_blob("log1", 0444, main_dir,
            &log1_blob);
    if (!file_log1){
        printk(KERN_ALERT "dtc: log1 file failed!\n");
        return -1;
    }
    return 0;
}

/* destructor */
void dtc_cleanup_debugfs(void){
    debugfs_remove_recursive(main_dir);
    return;
}

/* timestamp -> log1 */    
#define TIME_MSG_SIZE    (DTC_DEC_32 + 1 + DTC_DEC_64 + 1 + DTC_DEC_64 + 2 )
static u8 time_msg[TIME_MSG_SIZE];
static u32 time_msg_len = 0; 
void dtc_log_time(u32 time_loc){
    // time_loc time_sec time_usec
   
    if (log1_buf_pos + TIME_MSG_SIZE >= log1_buf_size) return;

    do_gettimeofday(&timestamp);
    time_msg_len = snprintf(time_msg, TIME_MSG_SIZE, "%u %lu %lu\n",
            time_loc, timestamp.tv_sec, timestamp.tv_usec);
    memcpy((char*)log1_blob.data+log1_buf_pos, time_msg, time_msg_len);
    
    log1_buf_pos += time_msg_len;
    log1_blob.size = log1_buf_pos;
    return;
}
// append data
#define U32_MSG_SIZE    ( DTC_DEC_32 + 2 )
static u8 u32_msg[U32_MSG_SIZE];
static u32 u32_msg_len; 
void dtc_log_time_u32(u32 data){
   if (log1_buf_pos + U32_MSG_SIZE >= log1_buf_size) return;
    u32_msg_len = snprintf(u32_msg, U32_MSG_SIZE, "%u\n",
            ntohl(data));
    memcpy((char*)log1_blob.data+log1_buf_pos, u32_msg, u32_msg_len);

    log1_buf_pos += u32_msg_len;
    log1_blob.size = log1_buf_pos;
    return;
}
