/* 
 * simple udp server
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVER_PORT 50000
#define BUF_LEN     512

// debuf
#define PACKET_RECV_MOD 1000000

void die(const char *s){
    perror(s);
    exit(-1);
}

int main(int argc, char **argv){

    if (argc != 2){
        printf("\t Error \n");
        printf("\t Format: program server_ip \n");
        exit(-1);
    }

    struct sockaddr_in si_me, si_you;

    int s, slen = sizeof(si_me), recv_len;
    char buf[BUF_LEN];

    unsigned long packet_recv = 0;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket()");
    }

    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, argv[1], &si_me.sin_addr.s_addr) != 1){
        die("inet_pton()");
    }

    if (bind(s, (struct sockaddr*) &si_me, sizeof(si_me)) == -1){
        die("bind()");
    }

    printf("everything good, start listening on port %d \n", SERVER_PORT);
    
    while(1){
        
        if ((recv_len = recvfrom(s, buf, BUF_LEN, 0, (struct sockaddr *) &si_you, &slen)) == -1){
            die("recvfrom()");
        }

        packet_recv++;
        
        if (packet_recv % PACKET_RECV_MOD == 0){
            printf("packet received %ld M \n", (unsigned long) (packet_recv / 1e6) );
        }


    }

    close(s);
    return 0;

}
