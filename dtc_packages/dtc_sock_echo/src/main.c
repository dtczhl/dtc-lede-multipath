/* 
 * simple udp server
 *        echo back
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

#define BUF_LEN 5000
char buf[BUF_LEN];

struct sockaddr_in si_me, si_you;

unsigned int decimator = 0;

void die(const char *s){
    perror(s);
    exit(-1);
}

void printFormat(void){
	printf("Format: program -i IP_address -p Port_number -d Decimation_left_shift \n");
	printf("	-i (required)	ip address \n");
	printf("	-p (required)	port number \n");
	printf("	-d (optional)	decimation left shift of 1, default:31 \n");
}

void argumentProcess(int argc, char **argv){
	// initialize
	memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-i") == 0){ //ip address
			if (i < argc-1){
				i++;
				if (inet_pton(AF_INET, argv[i], &si_me.sin_addr.s_addr) != 1){
					die("inet_pton()");
				}	
			} else {
				die("-i option processing error");
			}
		} else if (strcmp(argv[i], "-p") == 0){ // port number
			if (i < argc-1){
				i++;
				si_me.sin_port = htons((unsigned short)(atoi(argv[i])));
			} else {
				die("-p option processing error");
			}
		} else if (strcmp(argv[i], "-d") == 0){ // debug print, decimation
			if (i < argc-1){
				i++;
				decimator = 1 << (atoi(argv[i]));
			} else {
				die("-d option processing error");
			}
		}
	}
	// check arguments
	if (si_me.sin_addr.s_addr == 0 || si_me.sin_port == 0){
		die("ip address and port number must not be 0");
	}

	memset(buf, 0, sizeof(buf));

	if (decimator == 0){
		decimator = 1 << 31;	
	}

	printf("decimation: %u\n", decimator);
}

int main(int argc, char **argv){

    if (argc < 2){
        printf("**** Error \n");
        printFormat();
        exit(-1);
    }

	argumentProcess(argc, argv);

    int s, slen = sizeof(si_me), recv_len;

    unsigned int packet_recv = 0;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket()");
    }

    if (bind(s, (struct sockaddr*) &si_me, sizeof(si_me)) == -1){
        die("bind()");
    }

    printf("Everything good, listening on port %d \n", ntohs(si_me.sin_port));
    
    while(1){
        
        if ((recv_len = recvfrom(s, buf, BUF_LEN, 0, (struct sockaddr *) &si_you, &slen)) == -1){
            die("recvfrom()");
        }

        packet_recv++;
 
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_you, slen) == -1) {
            die("sendto()");
        }
       
        if (packet_recv % decimator == 0){
            printf("packet recv %u\n", packet_recv);
        }
    }

    close(s);
    return 0;

}
