#include "mynet.h"
#include "chat.h"
#include <stdlib.h>
#include <unistd.h>

/* サーバーの本体部分、主に初期化やクライアントとの接続を行う */
void chat_server(int port_number, int n_client)
{
  int sock_listen;

  /* サーバの初期化 */
  sock_listen = init_tcpserver(port_number, 5);

  /* クライアントの接続 */
  init_client(sock_listen, n_client);

  close(sock_listen);

  /* メインループ */
  chat_loop();

}