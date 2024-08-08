#include "sping.h"

void send_ping(int sock, struct sockaddr_in server_adrs);
void recv_echo(int sock);
unsigned short calc_cksum(unsigned short *data, int len);

int main(int argc, char *argv[]) {
    struct hostent *server_host;
    struct sockaddr_in server_adrs;

    int sock;

    /* 引数のチェックと使用法の表示 */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s hostname\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* サーバ名をアドレス(hostent構造体)に変換する */
    if ((server_host = gethostbyname(argv[1])) == NULL) {
        exit_errmesg("gethostbyname()");
    }

    /* サーバの情報をsockaddr_in構造体に格納する */
    memset(&server_adrs, 0, sizeof(server_adrs));
    server_adrs.sin_family = AF_INET;
    memcpy(&server_adrs.sin_addr, server_host->h_addr, server_host->h_length);

    /* RAWモードでICMPプロトコルを指定しソケットを作成する */
    if ((sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
        exit_errmesg("socket()");
    }

    /* root権限を放棄する */
    setuid(getuid());

    /* ICMP ECHOパケットを送信 */
    send_ping(sock, server_adrs);

    /* ICMP ECHOパケットを受信 */
    recv_echo(sock);

    exit(EXIT_SUCCESS);
}

/* ICMP ECHOパケットを送信する
    sock: ソケット ディスクリプタ
    server_adrs: サーバのアドレス
*/
void send_ping(int sock, struct sockaddr_in server_adrs) {
    int len;
    char buf[BUFSIZE];
    struct icmp *packet;

    /* ICMPパケットの作成 */
    packet = (struct icmp *)buf;
    packet->icmp_type = ICMP_ECHO;
    packet->icmp_code = 0;
    packet->icmp_id = getpid();
    packet->icmp_seq = 1;
    gettimeofday((struct timeval *)packet->icmp_data, NULL);

    /* check sumの計算 */
    len = 8 + DATALEN;
    packet->icmp_cksum = 0;
    packet->icmp_cksum = calc_cksum((u_short *)packet, len);

    /* パケットの送信 */
    sendto(sock, buf, len, 0, (struct sockaddr *)&server_adrs,
           sizeof(server_adrs));
}

/* ICMP ECHOパケットを受信する
    sock: ソケット ディスクリプタ
*/
void recv_echo(int sock) {
    int from_len, len, head_len, icmp_len, gotit;
    char buf[BUFSIZE];
    struct sockaddr_in from_adrs;
    struct ip *ip_packet;
    struct icmp *icmp_packet;

    gotit = 0;

    while (!gotit) { /* 目的のパケットを受信するまで繰り返し */

        /* パケットを受信 */
        from_len = sizeof(from_adrs);
        len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from_adrs,
                       (socklen_t *)&from_len);

        /* 受信パケットの解析 */
        ip_packet = (struct ip *)buf;
        head_len = ip_packet->ip_hl << 2; /* IPヘッダの長さ */

        icmp_packet = (struct icmp *)(buf + head_len); /* ICMPヘッダの先頭 */
        if ((icmp_len = len - head_len) <
            8) { /* ICMPパケットの長さが妥当か？ */
            exit_errmesg("icmp_len < 8");
        }

        if (icmp_packet->icmp_type ==
            ICMP_ECHOREPLY) { /* 目的のパケットのみ選ぶ */
            if (icmp_packet->icmp_id != getpid()) { /* 他人が送信したもの! */
                continue;
            }

            if (icmp_len < 16) { /* データが小さすぎる! */
                exit_errmesg("icmp_len < 16");
            }

            gotit = 1;
            printf("Receive ICMP echo.\n");
        }
    }
}

/*
   データのチェック・サムを計算する
    data: 計算対象となるデータ(へのポインタ)
    len:  データの長さ(byte)
*/
unsigned short calc_cksum(unsigned short *data, int len) {
    int nleft = len;
    int sum = 0;
    unsigned short *w = data;
    unsigned short answer = 0;

    /* word(16bit)単位で データを加算する */
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    /* データが奇数バイトの場合, 最終バイトの処理 */
    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff); /* 桁上がり分(16bit)を下位桁に加える */
    sum += (sum >> 16); /* さらに繰り上がった分を加える */
    answer = ~sum;      /* 1の補数をとる */

    return (answer);
}