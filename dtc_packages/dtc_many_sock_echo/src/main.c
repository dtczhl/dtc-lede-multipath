/* 
 * udp many sock server
 *        echo back
 *
 * Huanle Zhang at UC Davis
 * www.huanlezhang.com
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

#define RECV_BUF_LEN 5000
static unsigned char recvBuf[RECV_BUF_LEN];

#define MAX_PAIR 5
static struct sockaddr_in *si_mes;
static struct sockaddr_in si_you;
static int selfSeq = 0;

unsigned int decimator = 0;

// save data to file
const char autoIdFilePath[] = "/root/autoId";
const char dataFileDir[] = "/dtc";
const char dataFileName[] = "dtcManySockEcho";


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
	printf("Format: program -s self_ip self_port -d Decimation_left_shift -dRecv \n");
	printf("Note: at most %d selfs \n", MAX_PAIR);
	printf("	-s (required)		ip port; can specify multiple \n");
	printf("	-dRecv (optional)	save recv packets data\n");
	printf("	-dSend (optional)	save send packets data\n");
	printf("	-d (optional)		decimation left shift of 1, default:31 \n");
}

void argumentProcess(int argc, char **argv){
	
	// initialize
	si_mes = (struct sockaddr_in*) malloc(MAX_PAIR * sizeof(struct sockaddr_in));
	memset((char *) si_mes, 0, sizeof(si_mes));
	for (int i = 0; i < MAX_PAIR; i++){
		si_mes[i].sin_family = AF_INET;
	}

    memset((char *) &si_you, 0, sizeof(si_you));

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-s") == 0){ //ip port
			i++;
			if (inet_pton(AF_INET, argv[i], &si_mes[selfSeq].sin_addr.s_addr) != 1){
				die("inet_pton()");
			}
			i++;
			si_mes[selfSeq].sin_port = htons((unsigned short)(atoi(argv[i])));
			selfSeq++;

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
	if (selfSeq == 0){
		die("No self");
	}

	memset((char *)recvBuf, 0, sizeof(recvBuf));

	if (decimator == 0){
		decimator = 1 << 31;	
	}

	printf("decimation: %u\n", decimator);
	
	for (int i = 0; i < selfSeq; i++){
		printf(" self: %s port: %d \n",
				inet_ntoa(si_mes[i].sin_addr), ntohs(si_mes[i].sin_port));
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
			die("cannot open file for write");
		}
	}
}

static int sock[MAX_PAIR];
static int sock_len[MAX_PAIR];

static unsigned int packet_recv = 0;
static int maxfd = 0;
fd_set rset;

int main(int argc, char **argv){
	

    if (argc < 2){
        printf("**** Error \n");
        printFormat();
        exit(-1);
    }

	int slen, recv_len = 0;
	
	argumentProcess(argc, argv);

	for (int i = 0; i < selfSeq; i++){
		sock_len[i] = sizeof(si_mes[i]);
		if ((sock[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
			die("socket()");
		}
		if(bind(sock[i], (struct sockaddr*) &si_mes[i], sock_len[i]) < 0){
			die("bind");
		}
	}

    for (int i = 0; i < selfSeq; i++){
		if (sock[i] > maxfd){
			maxfd = sock[i];
		}
	}
	
	dataFileProcess(); // configure file for data
	
    printf("Everything good\n");
	for (int i = 0; i < selfSeq; i++){
		printf(" listening %s port: %d \n", 
				inet_ntoa(si_mes[i].sin_addr), ntohs(si_mes[i].sin_port));
	}
    
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
				} 

				if (bRecvData){
					gettimeofday(&tv, NULL);
					fprintf(fRecvDataFile, "%u %lu %lu %u-%u-%u-%u-\n", 
								si_you.sin_addr.s_addr,
								tv.tv_sec, tv.tv_usec,
								recvBuf[0], recvBuf[1], recvBuf[2], recvBuf[3]);
				}

				packet_recv++;
				if (packet_recv % decimator == 0){
					printf("total packet recv count: %u  From %s %d \n", packet_recv, 
							inet_ntoa(si_you.sin_addr), 
							ntohs(si_you.sin_port));
				}

				if (sendto(sock[i], recvBuf, recv_len, 0, 
								(struct sockaddr*) &si_you, slen) == -1){
					die("sendto()");
				}
			}
		}
		// fluash file
		if (bRecvData){
			fflush(fRecvDataFile);
		}
    }

	for (int i = 0; i < selfSeq; i++){
		close(sock[i]);
	}

	if (bRecvData){
		fclose(fRecvDataFile);
	}

    return 0;
}
