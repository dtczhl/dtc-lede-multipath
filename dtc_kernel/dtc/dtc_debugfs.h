#ifndef _DTC_DEBUGFS_H_
#define _DTC_DEBUGFS_H_

#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/string.h>
#include <linux/timekeeping.h>
#include <linux/ieee80211.h>

/* protocol */
#define DTC_TCP  ( 1 << 0 )
#define DTC_UDP  ( 1 << 1 )

/* timekeeping location */
#define DTC_TIME_SOCK   ( 1 << 0 )
#define DTC_TIME_TCP    ( 1 << 1 )
#define DTC_TIME_UDP    ( 1 << 2 )
#define DTC_TIME_MAC    ( 1 << 3 )
#define DTC_TIME_ATH    ( 1 << 4 )

/* byte size for APU2 */
#define DTC_CHAR_SIZE   1
#define DTC_INT_SIZE    4
#define DTC_LONG_SIZE   8

/* length: decimals to string literals */
#define DTC_DEC_32  10
#define DTC_DEC_64  20

/* variables */
extern u32 dtc_debugfs_enable;
extern u32 dtc_debugfs_time_loc; 
extern u32 dtc_debugfs_target_ip;
extern u16 dtc_debugfs_target_port;

/* functions */
int dtc_init_debugfs(char *dirname);
void dtc_cleanup_debugfs(void);
void dtc_log_time(u32 time_loc);
void dtc_log_time_u32(u32 data);

/* inline functions */
// point to udp data payload
static inline unsigned char *dtc_skb_udp_payload(const struct sk_buff *skb){
    return skb_transport_header(skb) + 8;
}
// ieee80211 data frame
static inline int dtc_is_ieee80211_data(const struct sk_buff *skb, const __le16 frame_control){
// udp + ip + ieee80211
#define MIN_FRAME ( 8 + 20 + 10 )
    if (skb->data_len < MIN_FRAME ) return 0;
    return (frame_control & 0x000F) == IEEE80211_FTYPE_DATA;
}


#endif
