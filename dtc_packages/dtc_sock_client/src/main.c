/* 
 * simple udp client
 * 
 * Huanle Zhang
 * www.huanlezhang.com
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define SERVER_PORT 50000
#define BUF_LEN     508 // maximum safe buffer size 

#define LONG_SIZE   8

// debug
#define PACKET_SEND_MOD 100000

void die(const char *s){
    perror(s);
    exit(-1);
}

int main(int argc, char **argv){

    if (LONG_SIZE != sizeof(long)){
        printf("\t Careful!!! long size = %ld \n", sizeof(long));
        exit(-1);
    }

    if (argc < 2){
        printf("\t Error \n");
        printf("\t Format: program server_ip [delay(us)]\n");
        exit(-1);
    }

    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 0;
    if (argc > 2){
        delay.tv_nsec = 1000 * atoi(argv[2]);
    }
    printf("delay: %lu (ns)\n", delay.tv_nsec);

    struct sockaddr_in si_me, si_you;

    int s, slen = sizeof(si_me);
    char buf[BUF_LEN];
    unsigned int seq = 0;
    
    unsigned int packet_send = 0;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket()");
    }

    memset((char *) &si_you, 0, sizeof(si_you));
    si_you.sin_family = AF_INET;
    si_you.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, argv[1], &si_you.sin_addr.s_addr) != 1){
        die("inet_pton()");
    }

    memset(buf, 0, sizeof(buf));


    while (1){

        seq++;

        buf[0] = (char) (seq >> 24);
        buf[1] = (char) (seq >> 16);
        buf[2] = (char) (seq >> 8);
        buf[3] = (char) (seq >> 0);
    
        if (sendto(s, buf, BUF_LEN, 0, (struct sockaddr *) &si_you, slen) == -1){
            die("sendto");
        }

        packet_send++;
        
        if (packet_send % PACKET_SEND_MOD == 0) {
            printf("packet send: %ld 100k \n", (unsigned long) (packet_send / 100000));
        }

        nanosleep(&delay, NULL);
    }

    close(s);
    return 0;

}    
    

