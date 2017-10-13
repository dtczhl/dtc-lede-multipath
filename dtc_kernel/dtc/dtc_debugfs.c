#include "dtc_debugfs.h"

/* main directory */
static struct dentry *main_dir;
static u8 nLogFile = 0; // how many log files we need
static u8 ratio_long2int = 0; // 
static u8 ratio_int2byte = 0;
static u8 ratio_long2byte = 0; // 

/* enable */
static struct dentry *file_enable;
u32 dtc_debugfs_enable = 0;

/* timestamp */
static struct dentry *file_time_loc;
u32 dtc_debugfs_time_loc = 0;
static struct timeval timestamp1; // split, send path? 
static struct timeval timestamp2; //		recv path?

/* target */
static struct dentry *file_target;
static char target_ip_port[24] = "1.1.1.1 1\n";
u32 dtc_debugfs_target_ip = 0;
u16 dtc_debugfs_target_port = 0;

/* info */
static struct dentry *file_info;
#define INFO_BUF_SIZE   1024
static u8 info_buf[INFO_BUF_SIZE]; // make it simple, output static info only
static u64 info_buf_pos = 0;				// otherwise, use log file

/* log1 in binary format */
static struct dentry *file_log1;
static const u64 log1_buf_size = 40 * 1024 * 1024; // change this to be big enough
static u64 log1_buf_pos = 0;
static struct debugfs_blob_wrapper log1_blob;

/* log2 in binary format */
static struct dentry *file_log2;
static const u64 log2_buf_size = 40 * 1024 * 1024; // change this to be big enough
static u64 log2_buf_pos = 0;
static struct debugfs_blob_wrapper log2_blob;

/* -------- implementation -------- */

/* ---- info ---- */
void dtc_debugfs_add_info(char *pInfo){
	if (info_buf_pos + strlen(pInfo) >= INFO_BUF_SIZE){
		return; // need improve
	}

	memcpy(info_buf + info_buf_pos, pInfo, strlen(pInfo));
	info_buf_pos += strlen(pInfo);
	return; 
}

static ssize_t info_read_file(struct file *file, char __user *user_buf,
        size_t count, loff_t *ppos){
    /*
	int buf_len = snprintf(info_buf, INFO_BUF_SIZE, // !!! change INFO_BUF_SIZE correspondingly
            "HZ: %-4d\n"
            , HZ);
	*/
    return simple_read_from_buffer(user_buf, count, ppos, info_buf, info_buf_pos);
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
    if (nLogFile >= 1){
        log1_buf_pos = 0;
        log1_blob.size = 0;
    }
    if (nLogFile >= 2){
        log2_buf_pos = 0;
        log2_blob.size = 0;
    }
    
    return count;
}
static struct file_operations target_fops = {
    .read = target_read_file,
    .write = target_write_file,
};




/* initialization */
int dtc_init_debugfs(char *dirname, int buffer_num){
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

	/* ---- other main initialization --- */
    nLogFile = buffer_num;
	ratio_long2int = (u8) (sizeof(long) / sizeof(int));
	ratio_long2byte = (u8) (sizeof(long) / sizeof(int));
	ratio_int2byte = sizeof(int);

	/* add info */
	dtc_debugfs_add_info("HZ=Something\n");

	/* initialize log files at below */
    if (nLogFile == 0) return 0;
    
	/* log1 */
    log1_blob.data = vmalloc(log1_buf_size);
    log1_blob.size = 0;
    file_log1 = debugfs_create_blob("log1", 0444, main_dir,
            &log1_blob);
    if (!file_log1){
        printk(KERN_ALERT "dtc: log1 file failed!\n");
        return -1;
    }
    if (nLogFile == 1) return 0;

    /* log2 */
    log2_blob.data = vmalloc(log2_buf_size);
    log2_blob.size = 0;
    file_log2 = debugfs_create_blob("log2", 0444, main_dir,
            &log2_blob);
    if (!file_log2){
        printk(KERN_ALERT "dtc: log2 file failed!\n");
        return -1;
    }
    if (nLogFile == 2) return 0;

    return 0;
}

/* destructor */
void dtc_cleanup_debugfs(void){
    debugfs_remove_recursive(main_dir);
    return;
}

/* log1 */
// format: length(4) timeval(long+long) data
void dtc_debugfs_log1(u32 *pData, u32 length){
	if (log1_buf_pos + length + 4 + 2 * ratio_long2byte // length timeval
			>= log1_buf_size) return; // run out buffer, simply return, need improve
	
	memcpy((u8*)log1_blob.data+log1_buf_pos, &length, 4); // length
	log1_buf_pos += 4;
	do_gettimeofday(&timestamp1);
	memcpy((u8*)log1_blob.data+log1_buf_pos, &timestamp1, 2*ratio_long2byte); // timestamp: 2 long
	log1_buf_pos += 2*ratio_long2byte;
	memcpy((u8*)log1_blob.data+log1_buf_pos, pData, length);
	log1_buf_pos += length;
	log1_blob.size = log1_buf_pos;
}

/* log2 */
// format: length(4) timeval(long+long) data
void dtc_debugfs_log2(u32 *pData, u32 length){
	if (log2_buf_pos + length + 4 + 2 * ratio_long2byte // timeval
			>= log2_buf_size) return; // run out buffer, simply return, need improve
	
	memcpy((u8*)log2_blob.data+log2_buf_pos, &length, 4);
	log2_buf_pos += 4;
	do_gettimeofday(&timestamp2);
	memcpy((u8*)log2_blob.data+log2_buf_pos, &timestamp2, 2*ratio_long2byte); // timestamp: 2 long
	log2_buf_pos += 2*ratio_long2byte;
	memcpy((u8*)log2_blob.data+log2_buf_pos, pData, length);
	log2_buf_pos += length;
	log2_blob.size = log2_buf_pos;
}


/* timestamp1 -> log1 */  
/*
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
// append data
#define INT_MSG_SIZE    ( DTC_DEC_32 + 2 )
static u8 int_msg[INT_MSG_SIZE];
static u32 int_msg_len;
void dtc_log_buffer_size(int data){
    if (log1_buf_pos + INT_MSG_SIZE >= log1_buf_size) return;
    int_msg_len = snprintf(int_msg, INT_MSG_SIZE, "%u\n",
            data);
    memcpy((char*)log1_blob.data+log1_buf_pos, int_msg, int_msg_len);

    log1_buf_pos += int_msg_len;
    log1_blob.size = log1_buf_pos;
    return;
}
*/
/* timestamp -> log2 */  
/*
#define TIME_MSG2_SIZE    (DTC_DEC_32 + 1 + DTC_DEC_64 + 1 + DTC_DEC_64 + 2 )
static u8 time_msg2[TIME_MSG2_SIZE];
static u32 time_msg2_len = 0; 
void dtc_log2_time(u32 time_loc){
    // time_loc time_sec time_usec
   
    if (log2_buf_pos + TIME_MSG2_SIZE >= log2_buf_size) return;

    do_gettimeofday(&timestamp2);
    time_msg2_len = snprintf(time_msg2, TIME_MSG2_SIZE, "%u %lu %lu\n",
            time_loc, timestamp2.tv_sec, timestamp2.tv_usec);
    memcpy((char*)log2_blob.data+log2_buf_pos, time_msg2, time_msg2_len);
    
    log2_buf_pos += time_msg2_len;
    log2_blob.size = log2_buf_pos;
    return;
}
// append data
#define U32_MSG2_SIZE    ( DTC_DEC_32 + 2 )
static u8 u32_msg2[U32_MSG2_SIZE];
static u32 u32_msg2_len; 
void dtc_log2_time_u32(u32 data){
   if (log2_buf_pos + U32_MSG2_SIZE >= log2_buf_size) return;
    u32_msg2_len = snprintf(u32_msg2, U32_MSG2_SIZE, "%u\n",
            ntohl(data));
    memcpy((char*)log2_blob.data+log2_buf_pos, u32_msg2, u32_msg2_len);

    log2_buf_pos += u32_msg2_len;
    log2_blob.size = log2_buf_pos;
    return;
}
*/
