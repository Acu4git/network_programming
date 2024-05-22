/**
 * [使い方]
 *
 * (1)プロキシ環境下でない場合
 * ./http_client http://www.is.kit.ac.jp/
 * ./http_client http://abehiroshi.la.coocan.jp/
 *
 * (2)プロキシ環境下(CISのプロキシサーバ)
 * ./http_client http://www.is.kit.ac.jp/ cis.kit.ac.jp 8080
 * ./http_client http://abehiroshi.la.coocan.jp/ cis.kit.ac.jp 8080
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
#define BUFSIZE 16384 /* バッファサイズ */

int isUnderProxy();
char *extractHostName(char *);
char *extractPath(char *);

char *getHTTPStatus(const char *);
int getHTTPStatusCode(const char *);
char *getHTTPStatusMessage(const char *);
int getContentLength(const char *);
char *getServerName(const char *);

int main(int argc, char *argv[]) {
  /*プロキシ環境かどうか確認*/
  if (!isUnderProxy() && argc != 2) {
    fprintf(stderr, "Error : invalid arguments without proxy\n");
    fprintf(stderr, "Usage : ./http_client <target_server>\n");
    exit(1);
  }
  if (isUnderProxy() && argc != 4) {
    fprintf(stderr, "Error : invalid arguments under proxy\n");
    fprintf(
        stderr,
        "Usage : ./http_client <target_server> <proxy_server> <proxy_port>\n");
    exit(1);
  }

  struct hostent *server_host;
  struct sockaddr_in server_adrs;

  char s_buf[BUFSIZE], r_buf[BUFSIZE];
  int strsize;

  int tcpsock, port = PORT;
  char *servername, *proxyname = NULL, *url = argv[1], *path;
  if ((servername = extractHostName(url)) == NULL) {
    fprintf(stderr, "Error : cannot find server name\n");
    exit(1);
  }
  path = extractPath(url);

  // printf("servername = %s\n", servername);

  if (argc == 4) {
    proxyname = argv[2];   // プロキシサーバ名
    port = atoi(argv[3]);  // プロキシポート
  }

  /* サーバ名をアドレス(hostent構造体)に変換する */
  switch (argc) {
    case 2:
      server_host = gethostbyname(servername);
      break;
    case 4:
      server_host = gethostbyname(proxyname);
      break;
  }
  if (server_host == NULL) {
    fprintf(stderr, "gethostbyname()");
    exit(EXIT_FAILURE);
  }

  /* サーバの情報をsockaddr_in構造体に格納する */
  memset(&server_adrs, 0, sizeof(server_adrs));
  server_adrs.sin_family = AF_INET;
  server_adrs.sin_port = htons(port);
  memcpy(&server_adrs.sin_addr, server_host->h_addr_list[0],
         server_host->h_length);

  /* ソケットをSTREAMモードで作成する */
  if ((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "socket()");
    exit(EXIT_FAILURE);
  }

  /* ソケットにサーバの情報を対応づけてサーバに接続する */
  if (connect(tcpsock, (struct sockaddr *)&server_adrs, sizeof(server_adrs)) ==
      -1) {
    fprintf(stderr, "connect");
    exit(EXIT_FAILURE);
  }

  switch (argc) {
    case 2:
      snprintf(s_buf, BUFSIZE,
               "GET %s HTTP/1.1\r\n"
               "HOST: %s\r\n",
               path, servername);
      break;
    case 4:
      snprintf(s_buf, BUFSIZE,
               "GET %s HTTP/1.1\r\n"
               "HOST: %s\r\n",
               url, servername);
      break;
  }

  strsize = strlen(s_buf);

  /* 文字列をサーバに送信する */
  if (send(tcpsock, s_buf, strsize, 0) == -1) {
    fprintf(stderr, "send()");
    exit(EXIT_FAILURE);
  }

  send(tcpsock, "\r\n", 2, 0);

  /* サーバから文字列を受信する */
  if ((strsize = recv(tcpsock, r_buf, BUFSIZE, 0)) == -1) {
    fprintf(stderr, "recv()");
    exit(EXIT_FAILURE);
  }
  r_buf[strsize] = '\0';

  // printf("%s\n", r_buf); /* 受信した文字列を画面に書く */

  char *httpStatus = getHTTPStatus(r_buf);
  int statusCode = getHTTPStatusCode(httpStatus);
  char *message = getHTTPStatusMessage(httpStatus);

  int errFlg = (statusCode / 100) == 4 || (statusCode / 100) == 5;
  if (errFlg) {
    printf("\033[33m");
    switch (statusCode / 100) {
      case 4:
        printf("Client-Side Error!\n");
        break;
      case 5:
        printf("Server-Side Error!\n");
        break;
    }
  } else {
    printf("\033[32m");
    printf("Success!\n");
  }
  printf("\033[0m");
  printf("Status Code: %d\n", statusCode);
  printf("Status Comment: %s\n", message);
  putchar('\n');

  printf("Content-Length: %d\n", getContentLength(r_buf));
  printf("Server: %s\n", getServerName(r_buf));

  close(tcpsock); /* ソケットを閉じる */
  exit(EXIT_SUCCESS);
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

// http://www.hogehoge.co.jp/
// のようなURLからホスト部(www.hogehoge.co.jp)を取り出す
char *extractHostName(char *url) {
  char *res = NULL;
  int i = 0, protcol_len = 0, len = 0, flag = 0, start;
  while (url[i] != '\0') {
    if (i > 3 && url[i - 2] == '/' && url[i - 1] == '/') {
      flag = 1;
      protcol_len = i;
    }
    if (flag) {
      if (url[i] == '/' || url[i] == '\0') break;
      if (len == 0) start = i;
      len++;
    }
    i++;
  }
  res = (char *)malloc(sizeof(char) * (len + 1));
  for (int m = start; m < start + len; m++) res[m - protcol_len] = url[m];
  res[len] = '\0';
  return res;
}

// http://www.hogehoge.co.jp/directory/file
// のようなURLからパス部(/directory/file)を抽出する
char *extractPath(char *url) {
  char *path = "/";
  int i = 0, cnt = 0;
  while (url[i] != '\0') {
    if (url[i] == '/') cnt++;
    if (cnt == 3) {
      path = &url[i];
      break;
    }
    i++;
  }

  return path;
}

// HTTPレスポンスのステータスを取得する
char *getHTTPStatus(const char *str) {
  int len = 0;
  const char *prefix = "HTTP/1.1";
  char *pos, *res;
  if ((pos = strstr(str, prefix)) == NULL) return NULL;
  pos += strlen(prefix) + 1;
  while (*pos != '\r') {
    pos++;
    len++;
  }
  pos -= len;

  res = (char *)malloc(sizeof(char) * (len + 1));
  snprintf(res, len + 1, "%s", pos);
  return res;
}

// レスポンスステータスからステータスコードを取得する
int getHTTPStatusCode(const char *statusStr) {
  int code = 0;
  while (*statusStr != ' ') {
    code *= 10;
    code += (int)((*statusStr) - '0');
    statusStr++;
  }
  return code;
}

// レスポンスステータスからステータスの詳細を取得する
char *getHTTPStatusMessage(const char *statusStr) {
  int len = 0;
  char *res = NULL;
  statusStr += 4;
  // printf("%s\n", statusStr);
  while (*statusStr != '\n') {
    statusStr++;
    len++;
  }
  statusStr -= len;

  res = (char *)malloc(sizeof(char) * (len + 1));
  snprintf(res, len, "%s", statusStr);
  return res;
}

// Content-Lengthヘッダの情報を取得
int getContentLength(const char *str) {
  int len = 0;
  const char *key = "Content-Length";
  char *pos = strstr(str, key);
  if (pos == NULL) return -1;

  pos += strlen(key) + 2;
  while (*pos != '\r') {
    len *= 10;
    len += (int)((*pos) - '0');
    pos++;
  }

  return len;
}

// Serverヘッダーの情報を取得
char *getServerName(const char *str) {
  int len = 0;
  const char *key = "Server";
  char *pos, *res;
  if ((pos = strstr(str, key)) == NULL) return NULL;
  pos += strlen(key) + 2;
  while (*pos != '\r') {
    pos++;
    len++;
  }
  pos -= len;

  res = (char *)malloc(sizeof(char) * (len + 1));
  snprintf(res, len, "%s", pos);
  return res;
}