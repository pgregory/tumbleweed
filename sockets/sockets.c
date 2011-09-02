#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

int Socket(const char* proto)
{
  int sockfd;
  struct protoent *protocol;

  /* protocol must be a number when used with socket() */
  protocol = getprotobyname(proto);
  sockfd = socket(AF_INET, SOCK_STREAM, protocol->p_proto);

  return sockfd;
}

int Shutdown(int sockfd)
{
  int res = shutdown(sockfd, SHUT_RDWR);

  if(res < 0)
    res = errno;

  return res;
}

int Connect(int sockfd, const char* addr, const char* servicename, const char* protocol)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  /* Resolve the host name */
  if (!(hostaddr = gethostbyname(addr))) 
    return h_errno;

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;

  /* setup the servent struct using getservbyname */
  servaddr = getservbyname(servicename, protocol);
  socketaddr.sin_port = servaddr->s_port;

  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

  /* everything is setup, now we connect */
  if(connect(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) 
    return errno;

  return 0;
}

int Send(int sockfd, const char* data, int len, int flags)
{
  size_t ret;
  /* send our get request */
  if((ret = send(sockfd, data, len, flags)) < 0)
    return errno;

  return ret;
}

int Close(int sockfd)
{
  if(close(sockfd) < 0)
    return errno;

  return 0;
}


int Read(int sockfd, const char* buffer, int len)
{
  size_t res;
  if((res = read(sockfd, buffer, len)) < 0)
    return errno;

  return res;
}
  
#define GET "GET / HTTP/1.0\n\n"

char* testSocket(int sockfd, int port, char* addr)
{
  int bufsize;
  char host[50];
  char buffer[1024];
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  /* Resolve the host name */
  if (!(hostaddr = gethostbyname(addr))) 
  {
    return "Error resolving host.";
  }

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;

  /* setup the servent struct using getservbyname */
  servaddr = getservbyname("http", "tcp");
  socketaddr.sin_port = servaddr->s_port;

  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

  /* everything is setup, now we connect */
  if(connect(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) {
    return "Error connecting.";
  }

  /* send our get request for http */
  if (send(sockfd, GET, strlen(GET), 0) == -1) {
    return "Error sending data.";
  }

  /* read the socket until its clear then exit */
  while ( (bufsize = read(sockfd, buffer, sizeof(buffer) - 1))) {
    //write(1, buffer, bufsize);
  }

  return strdup(buffer);
}
