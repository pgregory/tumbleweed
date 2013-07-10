#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif

int Socket(const char* proto, int* errcode)
{
  int sockfd;
  struct protoent *protocol;

  /* protocol must be a number when used with socket() */
  protocol = getprotobyname(proto);
  sockfd = socket(AF_INET, SOCK_STREAM, protocol->p_proto);

  if(sockfd < 0)
    *errcode = errno;

#ifndef WIN32
//  fcntl(sockfd, F_SETFL, O_NONBLOCK);
#else
//  u_long iMode = 1;
//  ioctlsocket(sockfd, FIONBIO, &iMode);
#endif

  return sockfd;
}

int Shutdown(int sockfd, int* errcode)
{
#ifndef WIN32
  int res = shutdown(sockfd, SHUT_RDWR);
#else
  int res = shutdown(sockfd, SD_BOTH);
#endif

  if(res < 0)
    *errcode = errno;

  return res;
}

int Connect(int sockfd, const char* addr, const char* servicename, const char* protocol, int* errcode)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  /* Resolve the host name */
  if (!(hostaddr = gethostbyname(addr))) 
  {
    *errcode = h_errno;
    return -1;
  }

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;

  /* setup the servent struct using getservbyname */
  servaddr = getservbyname(servicename, protocol);
  socketaddr.sin_port = servaddr->s_port;

  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

  /* everything is setup, now we connect */
  if(connect(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) 
  {
    *errcode = errno;
    return -1;
  }

  return 0;
}

int Bind(int sockfd, const char* servicename, const char* protocol, int* errcode)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;

  /* setup the servent struct using getservbyname */
  servaddr = getservbyname(servicename, protocol);
  socketaddr.sin_port = servaddr->s_port;

  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

  /* everything is setup, now we connect */
  if(bind(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) 
  {
    *errcode = errno;
    return -1;
  }

  return 0;
}


int BindToPort(int sockfd, int port, int* errcode)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_addr.s_addr = INADDR_ANY;
  socketaddr.sin_port = htons(port);

  /* everything is setup, now we connect */
  if(bind(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) 
  {
    *errcode = errno;
    return -1;
  }

  return 0;
}


int Send(int sockfd, const char* data, int len, int flags, int* errcode)
{
  ssize_t ret;
  /* send our get request */
  if((ret = send(sockfd, data, len, flags)) < 0)
  {
    *errcode = errno;
    return -1;
  }

  return ret;
}

int Listen(int sockfd, int backlog, int* errcode)
{
  if(listen(sockfd, backlog) < 0)
  {
    *errcode = errno;
    return -1;
  }

  return 0;
}

int Accept(int sockfd, int* errcode)
{
  struct sockaddr_in socketaddr;
  int newsockfd;
#ifndef WIN32
  socklen_t client_len;
#else
  int client_len;
#endif

  /* clear and initialize client socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));

  newsockfd = accept(sockfd, (struct sockaddr*)&socketaddr, &client_len);

  if(newsockfd < 0)
    *errcode = errno;

  return newsockfd;
}


int Close(int sockfd, int* errcode)
{
  if(close(sockfd) < 0)
  {
    *errcode = errno;
    return -1;
  }

  return 0;
}


int Read(int sockfd, char* buffer, int len, int* errcode)
{
  ssize_t res;
  if((res = read(sockfd, buffer, len)) < 0)
    *errcode = errno;

  return res;
}

int eagain()
{
#ifndef WIN32
  return EAGAIN;
#else
  return WSAEWOULDBLOCK;
#endif
}
  
