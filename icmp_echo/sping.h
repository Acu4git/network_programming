#ifndef SPING_H_
#define SPING_H_
/*
  sping.h
*/

#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

#include "mynet.h"

#define BUFSIZE 128
#define DATALEN 54

#ifdef __CYGWIN__

#define ICMP_ECHOREPLY 0 /* Echo Reply			*/
#define ICMP_ECHO 8      /* Echo Request			*/

struct icmp {
    u_int8_t icmp_type;   /* type of message, see below */
    u_int8_t icmp_code;   /* type sub code */
    u_int16_t icmp_cksum; /* ones complement checksum of struct */
    u_int16_t icmp_id;
    u_int16_t icmp_seq;
    u_int8_t icmp_data[1];
};

#endif

#endif /* SPING_H_ */