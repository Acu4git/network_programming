#include "client.h"

#include <arpa/inet.h>
#include <sys/select.h>

#include "mynet.h"

int starts_with(const char *str, const char *prefix);
char *remove_prefix(const char *str, const char *prefix);

void run_client(const char *username, int port, struct sockaddr_in *from_adrs) {
    int tcp_sock;
    int strsize;
    char *servername, *packet;
    char msg_buf[MSG_BUFSIZE], r_buf[R_BUFSIZE], s_buf[S_BUFSIZE];

    fd_set mask, readfds;

    // HEREパケットを受け取ったサーバーの情報からホスト部を取り出す
    servername = inet_ntoa(from_adrs->sin_addr);
    tcp_sock = init_tcpclient(servername, port);

    // JOIN形式で送信
    packet = create_packet(msg_buf, JOIN, username);
    strsize = strlen(packet);
    Send(tcp_sock, packet, strsize, 0);

    // マスクの初期化
    FD_ZERO(&mask);
    FD_SET(0, &mask);
    FD_SET(tcp_sock, &mask);

    while (1) {
        readfds = mask;
        select(tcp_sock, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(0, &readfds)) {
            /* キーボードから文字列を入力する */
            fgets(s_buf, S_BUFSIZE, stdin);

            // QUITで切断処理
            if (strcmp(s_buf, "QUIT") == 0) {
                packet = create_packet(msg_buf, QUIT, NULL);
                strsize = strlen(packet);
                Send(tcp_sock, packet, strsize, 0);
                break;
            }

            // POST形式で送信
            packet = create_packet(msg_buf, POST, s_buf);
            strsize = strlen(packet);
            Send(tcp_sock, packet, strsize, 0);
        }

        if (FD_ISSET(tcp_sock, &readfds)) {
            /* サーバから文字列を受信する */
            strsize = Recv(tcp_sock, r_buf, R_BUFSIZE - 1, 0);
            r_buf[strsize] = '\0';

            // MESG形式で受信したときの処理
            if (starts_with(r_buf, "MESG "))
                printf("%s\n", remove_prefix(r_buf, "MESG "));
        }
    }

    close(tcp_sock);
}

int starts_with(const char *str, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    // strncmp で文字列の先頭を接頭語と比較
    return strncmp(str, prefix, prefix_len) == 0;
}

char *remove_prefix(const char *str, const char *prefix) {
    // 接頭語が文字列の先頭にあるか確認
    if (strstr(str, prefix) == str) {
        // 接頭語の長さを計算し、それ以降の文字列を返す
        return (char *)(str + strlen(prefix));
    }
    // 接頭語がない場合は元の文字列をそのまま返す
    return (char *)str;
}