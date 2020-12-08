#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define MYPORT 3333 /* default port */
#define MAX_CLNT 100
#define MAX_IP 30
#define QLEN 5
#define BUF_SIZE 100

void *handle_clnt(void* arg);
void send_msg(char* msg, int len);
void error_handling(char *msg);
char *serverState(int count);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	
	struct tm *t;
	time_t timer = time(NULL);
	t = localtime(&timer);
	
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(MYPORT);
    if (bind (serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0) {
        error_handling("bind() failed");
        exit(1);
    }
    /* Specify a size of request queue */
    if (listen(serv_sock, QLEN) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }
    printf("Server start!!\n");
    
	while(1) {
		t = localtime(&timer);
      	clnt_adr_sz = sizeof(clnt_adr);
      	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
      	pthread_mutex_lock(&mutx);
      	clnt_socks[clnt_cnt++] = clnt_sock;
      	pthread_mutex_unlock(&mutx);
      	pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
      	pthread_detach(t_id);
      	printf("Connected Client IP : %s ", inet_ntoa(clnt_adr.sin_addr));
      	printf("%d-%d-%d %d:%d)\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
      	printf("  chatter (%d/100\n", clnt_cnt);
	}
	close(serv_sock);
	return 0;
}

void send_msg(char* msg, int len) {
	pthread_mutex_lock(&mutx);
	for(int i=0; i<clnt_cnt; i++)
    	write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void *handle_clnt(void* arg) {
	int clnt_sock=*((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
		send_msg(msg, str_len);
	pthread_mutex_lock(&mutx);
	for(int i=0; i<clnt_cnt;i++) {
		if(clnt_sock == clnt_socks[i]) {
			while(i++ < clnt_cnt -1)
				clnt_socks[i] = clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

