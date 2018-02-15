/* 
 * udp client for mutiple socks
 *
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
#include <sys/select.h>
#include <sys/time.h>

#define SEND_BUF_LEN 2000
#define RECV_BUF_LEN 5000
static unsigned char sendBuf[SEND_BUF_LEN];
static unsigned char recvBuf[RECV_BUF_LEN];

static struct timespec delay;

#define MAX_PAIR 5
static struct sockaddr_in *si_mes;
static struct sockaddr_in *si_yous;
static int selfSeq = 0;
static int targetSeq = 0;

fd_set rset;

static unsigned int decimator = 0;

// save data to file
const char autoIdFilePath[] = "/root/autoId";
const char dataFileDir[] = "/dtc";
const char dataFileName[] = "dtcManySockClient";
char dataRecvFileRealPath[128];

int bRecvData = 0;
int bSendData = 0;
char dataSendFileRealPath[128];
char dataRecvFileRealPath[128];
FILE *fRecvDataFile = NULL;
FILE *fSendDataFile = NULL;

struct timeval tv;

void die(const char *s){
    perror(s);
    exit(-1);
}

void printFormat(void){
	printf("Format: program -s self_ip self_port -t target_ip target_port -n Interval(us) -d Decimation_left_shift -dRecv -dSend \n");
	printf("Note, at most %d selfs and %d targets \n", MAX_PAIR, MAX_PAIR);
	printf("	-s (required)		ip port; can specify multiple\n");
	printf("	-t (required)		ip port; can specify multiple\n");
	printf("	-dRecv (optional)	save recv packets data\n");
	printf("	-dSend (optional)	save send packets data\n");
	printf("	-n (optional)	packet interval in us, default: 0\n");
	printf("	-d (optional)	decimation left shift of 1, default: 31\n");
}

void argumentProcess(int argc, char **argv){

	// initialize
	delay.tv_sec = 0;
    delay.tv_nsec = 0;
	
	si_mes = (struct sockaddr_in*) malloc(MAX_PAIR * sizeof(struct sockaddr_in));
	si_yous = (struct sockaddr_in*) malloc(MAX_PAIR * sizeof(struct sockaddr_in)); 

	memset((char *) si_mes, 0, sizeof(si_mes));
	memset((char *) si_yous, 0, sizeof(si_yous));
	for (int i = 0; i < MAX_PAIR; i++){
		si_mes[i].sin_family = AF_INET;
		si_yous[i].sin_family = AF_INET;
	}

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-s") == 0){
			i++;
			if (inet_pton(AF_INET, argv[i], &si_mes[selfSeq].sin_addr.s_addr) != 1){
				die("inet_pton()");
			}
			i++;
			si_mes[selfSeq].sin_port = htons((unsigned short)(atoi(argv[i])));
			selfSeq++;
		} else if (strcmp(argv[i], "-t") == 0){
			i++;
			if (inet_pton(AF_INET, argv[i], &si_yous[targetSeq].sin_addr.s_addr) != 1){
				die("inet_pton()");
			}
			i++;
			si_yous[targetSeq].sin_port = htons((unsigned short)atoi(argv[i]));
			targetSeq++;
		} else if (strcmp(argv[i], "-n") == 0){ // packet interval
			if (i < argc-1){
				i++;
				delay.tv_nsec = 1000 * atoi(argv[i]);
			} else {
				die("-n option processing error");
			}
		} else if (strcmp(argv[i], "-d") == 0){ // debug print, decimation
			if (i < argc-1){
				i++;
				decimator = 1 << (atoi(argv[i]));
			} else {
				die("-d option processing error");
			}
		} else if (strcmp(argv[i], "-dSend") == 0){ // log send data
			bSendData = 1;
		} else if (strcmp(argv[i], "-dRecv") == 0){ // log recv data
			bRecvData = 1;
		}
	}

	// check arguments
	if (selfSeq == 0 || targetSeq == 0){
		die("No self or target");
	}

	memset((char *) sendBuf, 0, sizeof(sendBuf));
	memset((char *) recvBuf, 0, sizeof(recvBuf));

	if (decimator == 0){
		decimator = 1 << 31;
	}

	printf("delay: %lu (ns)\n", delay.tv_nsec);
	printf("decimation: %u\n", decimator);

	for (int i = 0; i < selfSeq; i++){
		printf(" self: %s port: %d \n",
				inet_ntoa(si_mes[i].sin_addr), ntohs(si_mes[i].sin_port));
	}
	for (int i = 0; i < targetSeq; i++){
		printf(" target: %s port: %d \n",
				inet_ntoa(si_yous[i].sin_addr), ntohs(si_yous[i].sin_port));
	}
}

void dataFileProcess(void){
	
	if (!bSendData && !bRecvData){ // no need to save data
		return;
	}

	FILE *fAutoId = NULL;
	int recordId = 0;

	// check autoId file exists or not
	if (access(autoIdFilePath, F_OK) != -1){
		// autoId exists
		fAutoId = fopen(autoIdFilePath, "r");
		if (fAutoId == NULL){
			die("autoId file open failed");
		}
		fscanf(fAutoId, "%d", &recordId);
		fclose(fAutoId);
	}

	snprintf(dataSendFileRealPath, sizeof(dataSendFileRealPath), "%s/%s_send_%d", dataFileDir, dataFileName, recordId);
	snprintf(dataRecvFileRealPath, sizeof(dataRecvFileRealPath), "%s/%s_recv_%d", dataFileDir, dataFileName, recordId);

	if (bRecvData){
		fRecvDataFile = fopen(dataRecvFileRealPath, "w");
		if (fRecvDataFile == NULL){
			die("cannot open file for fRecvDataFile");
		}
	}

	if (bSendData){
		fSendDataFile = fopen(dataSendFileRealPath, "w");
		if (fSendDataFile == NULL){
			die("cannot open file for fSendDataFile");
		}
	}

}
    
static int sock[MAX_PAIR];
static int sock_len[MAX_PAIR];

static unsigned int seq = 0;
static unsigned int packet_send = 0;

static unsigned int packet_recv = 0;

static int maxfd = 0;

void childProcess(void){
	
	int slen, recv_len = 0;
	struct sockaddr_in si_you;

	while(1){

		FD_ZERO(&rset);

		for (int i = 0; i < selfSeq; i++){
			FD_SET(sock[i], &rset);
		}

		select(maxfd+1, &rset, NULL, NULL, NULL);

		for (int i = 0; i < selfSeq; i++){
			if (FD_ISSET(sock[i], &rset)){
				if ((recv_len = recvfrom(sock[i], recvBuf, RECV_BUF_LEN, 0,
									(struct sockaddr *) &si_you, &slen)) == -1){
					die("recvfrom()");
				};

				if (bRecvData){
					gettimeofday(&tv, NULL);
					fprintf(fRecvDataFile, "%u %lu %lu %u-%u-%u-%u-\n",
								si_you.sin_addr.s_addr,
								tv.tv_sec, tv.tv_usec,
								recvBuf[0], recvBuf[1], recvBuf[2], recvBuf[3]);
				}

				packet_recv++;
				if (packet_recv % decimator == 0) {
					printf("total packet recv count: %u From %s %d \n", packet_recv,
								inet_ntoa(si_you.sin_addr),
								ntohs(si_you.sin_port));
				}
			}
		}
		if (bRecvData){
			fflush(fRecvDataFile);
		}
 	}
}

void parentProcess(void){
    
	while (1){

        seq++;
		
		sendBuf[0] = (char) (seq >> 24);
		sendBuf[1] = (char) (seq >> 16);
		sendBuf[2] = (char) (seq >> 8);
		sendBuf[3] = (char) (seq >> 0);
		
		nanosleep(&delay, NULL);

		for (int i = 0; i < targetSeq; i++){
			if (sendto(sock[i], sendBuf, SEND_BUF_LEN, 0,  // send same packet to all targets
						(struct sockaddr *) &si_yous[i], sock_len[i]) == -1){
				die("sendto");
			}	
		}

		if (bSendData){
			gettimeofday(&tv, NULL);
			fprintf(fSendDataFile, "%lu %lu %u-%u-%u-%u-\n",
							tv.tv_sec, tv.tv_usec,
							sendBuf[0], sendBuf[1], sendBuf[2], sendBuf[3]);
			fflush(fSendDataFile);
		}

        packet_send++;
        
        if (packet_send % decimator == 0) {
            printf("unique packet send: %u\n", packet_send);
        }

    }
}

int main(int argc, char **argv){

    if (argc < 2){
        printf("**** Error \n");
        printFormat();
        exit(-1);
    }

	argumentProcess(argc, argv);

	for (int i = 0; i < selfSeq; i++){
		sock_len[i] = sizeof(si_mes[i]);
		if ((sock[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
			die ("socket()");
		}
		printf("sock id %d\n", sock[i]);
		if (bind (sock[i], (struct sockaddr *) &si_mes[i], sock_len[i]) < 0){
			die ("bind() failed");
		} 
	}

	for (int i = 0; i < selfSeq; i++){
		if (sock[i] > maxfd){
			maxfd = sock[i];
		}
	}

	dataFileProcess();


// two processes: one for sending, the other for receiving
	pid_t pid = fork();
	if (pid == 0){
		childProcess();
	} else {
		parentProcess();	
	}

	if (bSendData){
		fclose(fSendDataFile);
	}

    return 0;
}    
    
