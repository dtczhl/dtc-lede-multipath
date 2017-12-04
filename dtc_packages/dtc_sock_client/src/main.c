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
#include <sys/types.h>

#define BUF_LEN 1000
static char buf[BUF_LEN];
static char recvBuf[BUF_LEN];

static struct timespec delay;

static struct sockaddr_in si_me, si_you;

static unsigned int decimator = 0;

void die(const char *s){
    perror(s);
    exit(-1);
}

void printFormat(void){
	printf("Format: program -i IP_address -p Port_number -n Interval(us) -d Decimation_left_shift \n");
	printf("	-i (required)	ip address\n");
	printf("	-p (required)	port number\n");
	printf("	-n (optional)	packet interval in us, default: 0\n");
	printf("	-d (optional)	decimation left shift of 1, default: 31\n");
}

void argumentProcess(int argc, char **argv){
	// initialize
	delay.tv_sec = 0;
    delay.tv_nsec = 0;
	memset((char *) &si_you, 0, sizeof(si_you));
    si_you.sin_family = AF_INET;
 
	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-i") == 0){ // ip address
			if (i < argc-1){
				i++;
			    if (inet_pton(AF_INET, argv[i], &si_you.sin_addr.s_addr) != 1){
					die("inet_pton()");
				}
			} else {
				die("-i option processing error");
			}
		} else if (strcmp(argv[i], "-p") == 0){ // port number
			if (i < argc-1){
				i++;
				si_you.sin_port = htons((unsigned short)(atoi(argv[i])));
			} else {
				die("-p option processing error");
			}
		} else if (strcmp(argv[i], "-n") == 0){ // packet interval
			if (i < argc-1){
				i++;
				delay.tv_nsec = 1000 * atoi(argv[i]);
			} else {
				die("-i option processing error");
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
	if (si_you.sin_addr.s_addr == 0 || si_you.sin_port == 0){
		die("ip address and port number must not be 0");
	}

    memset(buf, 0, sizeof(buf));

	if (decimator == 0){
		decimator = 1 << 31;
	}				
	
	printf("delay: %lu (ns)\n", delay.tv_nsec);
	printf("decimation: %u\n", decimator);

}
    
static int s, slen;
static unsigned int seq = 0;
static unsigned int packet_send = 0;

void childProcess(void){
	
	printf("in child process, waiting to consume packets\n");	
	while(recvfrom(s, recvBuf, BUF_LEN, 0, (struct sockaddr *) &si_you, &slen) != -1){
		; // just free up buffer
	}
}

void parentProcess(void){
    
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
        
        if (packet_send % decimator == 0) {
            printf("packet send: %u\n", packet_send);
        }

        nanosleep(&delay, NULL);
    }
}

int main(int argc, char **argv){

    if (argc < 2){
        printf("**** Error \n");
        printFormat();
        exit(-1);
    }

	argumentProcess(argc, argv);

	slen = sizeof(si_me);



    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("socket()");
    }

// two processes: one for sending, the other for receiving
	pid_t pid = fork();
	if (pid == 0){
		childProcess();
	} else {
		parentProcess();	
	}

    close(s);
    return 0;
}    
    
