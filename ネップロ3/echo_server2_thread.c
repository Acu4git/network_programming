#include "mynet.h"
#include <pthread.h>
#include "stddef.h"

#define BUFSIZE 500   /* バッファサイズ */
FILE *fp;

void * echo_thread(void *arg);
void server(int);

/* スレッド関数の引数 */
struct myarg {
	int sock; /* acceptしたソケット */
	int id;      /* スレッドの通し番号 */
};

int main(int argc, char *argv[]){

	int sock_listen;
	int i;
	struct myarg *tharg;
	pthread_t tid;

	/* 引数のチェックと使用法の表示 */
	if( argc != 3 ){
		fprintf(stderr,"Usage: %s Port_number\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* サーバの初期化 */
	sock_listen = init_tcpserver(atoi(argv[1]), 5);

	for(i=0; i<atoi(argv[2]); i++){

		/* スレッド関数の引数を用意する */
		if( (tharg = (struct myarg *)malloc(sizeof(struct myarg)))==NULL ){
			exit_errmesg("malloc()");
		}

		tharg->sock = sock_listen;
		tharg->id = i;

		/* スレッドを生成する */
		if( pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0 ){
			exit_errmesg("pthread_create()");
		}
	}
	pthread_exit(NULL);
}


/* スレッドの本体 */
void * echo_thread(void *arg){
	int sock_accepted;
	struct myarg *tharg;

	tharg = (struct myarg *)arg;

	pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

	for(;;){
	/* クライアントの接続を受け付ける */
	sock_accepted = accept(tharg->sock, NULL, NULL);

	server(sock_accepted);

	close(sock_accepted);   /* ソケットを閉じる */
	}
	return(NULL);
}

void server(int sock){
	int strsize;
	char buf[BUFSIZE];

	/* ">"を送信する */
	do{
		if(send(sock, ">", 1, 0) == -1 ){
			fprintf(stderr,"send()");
			exit(EXIT_FAILURE);
		}

		/* 文字列をクライアントから受信する */
		if((strsize=recv(sock, buf, BUFSIZE, 0)) == -1){
			exit_errmesg("recv()");
		}

		/* list処理 */
		if(strncmp(buf,"list\r\n",6)==0){
			fp = popen("ls ~/work", "r");
			while(fgets(buf, sizeof(buf),fp) != NULL){
				strcat(buf, "\n");
				if(send(sock, buf, strlen(buf),0) == -1){
					fprintf(stderr,"send()");
					exit(EXIT_FAILURE);
				}
			}
			pclose(fp);
		}

		/* type処理 */
		if(strncmp(buf,"type ",5)==0){
			char textname[BUFSIZE] = "\0";
			char command[]="cat /home/student/work/";
			char *name = strstr(buf, "type ");	
			strcat(command, textname+5);

			char text[]="This file is ";

			/* popenで使用するコマンド文字列作成 */
			strncpy(textname,buf+5,strlen(buf+5)-2);
			strncat(command,textname,strlen(buf+5)-2);

			fp =popen(command,"r");

			/* 送信したい文字列作成 */
			if(fgets(buf, sizeof(buf),fp) != NULL){
				strcat(text,textname);
				strcat(text,".\n\n");
				strcat(text,buf);
				strcat(text,"\n");

				if(send(sock, text, strlen(text),0) == -1){
					fprintf(stderr,"send()");
					exit(EXIT_FAILURE);
				}
				pclose(fp);
			}
		}
	}while(strncmp(buf,"exit\r\n",6)); /* 改行コードを受信するまで繰り返す */
}