/**
 * [Usage]
 *
 * ./tcp_client <target_server>
 * or
 * ./tcp_client <target_server> <proxy_server> <proxy_port>
 */
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 80
#define BUFSIZE 1024 /* バッファサイズ */

int isUnderProxy();
int getContentLength(char *);

int main(int argc, char *argv[]) {
  /*プロキシ環境かどうか確認*/
  if (!isUnderProxy() && argc != 2) {
    fprintf(stderr, "<ARGS ERROR without proxy>\n");
    fprintf(stderr, "Usage : ./tcp_client <target_server>\n");
    exit(1);
  }
  if (isUnderProxy() && argc != 4) {
    fprintf(stderr, "<ARGS ERROR under proxy>\n");
    fprintf(
        stderr,
        "Usage : ./tcp_client <target_server> <proxy_server> <proxy_port>\n");
    exit(1);
  }

  struct hostent *server_host;
  struct sockaddr_in server_adrs;

  int tcpsock, port = PORT;

  char *servername, *proxyname;
  servername = argv[1];
  proxyname = NULL;
  if (isUnderProxy()) {
    proxyname = argv[2];
    port = atoi(argv[3]);
  }

  char k_buf[BUFSIZE], s_buf[BUFSIZE], r_buf[BUFSIZE];
  int strsize;

  if (argc == 4) {
    /* サーバ名をアドレス(hostent構造体)に変換する */
    if ((server_host = gethostbyname(proxyname)) == NULL) {
      fprintf(stderr, "gethostbyname()");
      exit(EXIT_FAILURE);
    }

    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(&server_adrs, 0, sizeof(server_adrs));
    server_adrs.sin_family = AF_INET;
    server_adrs.sin_port = htons(PORT);
    memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0],
           server_host->h_length);

    /* ソケットをSTREAMモードで作成する */
    if ((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
      fprintf(stderr, "socket()");
      exit(EXIT_FAILURE);
    }

    /* ソケットにサーバの情報を対応づけてサーバに接続する */
    if (connect(tcpsock, (struct sockaddr *)&server_adrs,
                sizeof(server_adrs)) == -1) {
      fprintf(stderr, "connect");
      exit(EXIT_FAILURE);
    }

    /* キーボードから文字列を入力してサーバに送信 */
    while (fgets(k_buf, BUFSIZE, stdin),
           k_buf[0] != '\n') { /* 空行が入力されるまで繰り返し */
      strsize = strlen(k_buf);
      k_buf[strsize - 1] = 0; /* 末尾の改行コードを消す */
      snprintf(s_buf, BUFSIZE, "%s\r\n", k_buf); /* HTTPの改行コードは \r\n */
      printf("k_buf : %s\n", k_buf);

      /* 文字列をサーバに送信する */
      if (send(tcpsock, s_buf, strsize + 1, 0) == -1) {
        fprintf(stderr, "send()");
        exit(EXIT_FAILURE);
      }
    }

    send(tcpsock, "\r\n", 2, 0); /* HTTPのメソッド（コマンド）の終わりは空行 */

    /* サーバから文字列を受信する */
    if ((strsize = recv(tcpsock, r_buf, BUFSIZE - 1, 0)) == -1) {
      fprintf(stderr, "recv()");
      exit(EXIT_FAILURE);
    }
    r_buf[strsize] = '\0';

    printf("received: %s\n", r_buf); /* 受信した文字列を画面に書く */

    close(tcpsock); /* ソケットを閉じる */
    exit(EXIT_SUCCESS);
  }
}

// isUnderProxy関数は，プロキシ環境下であるときに1を返し，そうでない場合は0を返す
int isUnderProxy() {
  int res = 0;
  static char *envList[] = {"http_proxy", "https_proxy", "HTTP_PROXY",
                            "HTTPS_PROXY"};

  for (int i = 0; i < 4; i++) {
    if (getenv(envList[i]) != NULL) res = 1;
  }

  return res;
}

int getContentLentgth(char *res) {
  char *key = "Content-Length";
  return -1;
}