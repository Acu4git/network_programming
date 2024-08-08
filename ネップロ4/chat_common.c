#include "mynet.h"

/* acceptのエラー検査 */
int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int r;
  if((r=accept(s,addr,addrlen))== -1){
    exit_errmesg("accept()");
  }
  return(r);
}

/* sendのエラー検査 */
int send(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=send(s,buf,len,flags))== -1){
    exit_errmesg("send()");
  }
  return(r);
}

/* receiveのエラー検査 */
int recv(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=recv(s,buf,len,flags))== -1){
    exit_errmesg("recv()");
  }
  return(r);
}