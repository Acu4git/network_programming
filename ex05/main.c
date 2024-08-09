#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

#include "client.h"
#include "mynet.h"

#define USERNAME_MAXLEN 15
#define MAX_REPEAT 3

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <username> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock;                 // ソケット番号
    int broadcast_sw = 1;     // ブロードキャストの設定値
    int port = DEFAULT_PORT;  // ポートの設定
    if (argc == 3) port = atoi(argv[2]);

    struct sockaddr_in broadcast_adrs, from_adrs;
    socklen_t from_len;
    fd_set mask, readfds;
    struct timeval timeout;
    char username[USERNAME_MAXLEN], s_buf[S_BUFSIZE], r_buf[R_BUFSIZE];
    strncpy(username, argv[1], USERNAME_MAXLEN);
    int strsize;

    int count = 0;  // ブロードキャストの回数
    int server_exist = 0;

    // ブロードキャストアドレスの情報をsockaddr_in構造体に格納する
    set_sockaddr_in_broadcast(&broadcast_adrs, port);

    // ソケットをDGRAMモードで作成
    sock = init_udpclient();

    // ソケットをブロードキャスト可能にする
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_sw,
                   sizeof(broadcast_sw)) == -1) {
        exit_errmesg("setsockopt()");
    }

    // ビットマスクの準備
    FD_ZERO(&mask);
    FD_SET(sock, &mask);

    while (count < MAX_REPEAT && !server_exist) {
        // "HELO"パケットの送信
        char *packet = create_packet(s_buf, HELLO, NULL);
        Sendto(sock, packet, strlen(packet), 0,
               (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs));

        //  受信データをチェック
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int ret = select(sock + 1, &readfds, NULL, NULL, &timeout);
        if (ret > 0) {
            from_len = sizeof(from_adrs);
            strsize = Recvfrom(sock, r_buf, R_BUFSIZE - 1, 0,
                               (struct sockaddr *)&from_adrs, &from_len);
            r_buf[strsize] = '\0';
            // HEREが返ってきたかどうか
            if (strcmp(r_buf, "HERE") == 0) {
                server_exist = 1;
                break;
            }
        } else if (ret == -1) {
            close(sock);
            exit_errmesg("select()");
        }

        printf("Time out.[%d]\n", count + 1);
        count++;
    }

    close(sock);

    if (server_exist) {
        // クライアントとして動作
        run_client(username, port, &from_adrs);
    } else {
        // サーバーとして動作
        // run_server(username, port);
        fprintf(stderr, "Error: can't find a server.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}