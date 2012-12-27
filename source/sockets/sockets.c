#include "memory.h"
#include "primitive.h"
#include "names.h"

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

object PRIM_socketNew(int primitiveNumber, object* args, int argc)
{
  int sockfd;
  struct protoent *protocol;
  int yes=1;
  //char yes='1'; // Solaris people use this

  if(argc != 1)
    sysError("invalid number of arguments to primitive", "PRIM_socketNew");

  /* protocol must be a number when used with socket() */
  protocol = getprotobyname(charPtr(args[0]));
  sockfd = socket(AF_INET, SOCK_STREAM, protocol->p_proto);

  // lose the pesky "Address already in use" error message
  if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) 
      sysError("setsockopt", "PRIM_socketNew");
  
  //if(sockfd < 0)
  //  *errcode = errno;

#ifndef WIN32
//  fcntl(sockfd, F_SETFL, O_NONBLOCK);
#else
//  u_long iMode = 1;
//  ioctlsocket(sockfd, FIONBIO, &iMode);
#endif

  return newInteger(sockfd);
}

object PRIM_socketShutdown(int primitiveNumber, object* args, int argc)
{
  if(argc != 1)
    sysError("invalid number of arguments to primitive", "PRIM_sockectShutDown");

#ifndef WIN32
  int res = shutdown(getInteger(args[0]), SHUT_RDWR);
#else
  int res = shutdown(getInteger(args[0]), SD_BOTH);
#endif

  if(res == -1)
    return newInteger(errno);
  else
    return newInteger(0);
}

object PRIM_socketConnect(int primitiveNumber, object* args, int argc)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  int sockfd;
  const char *addr, *servicename, *protocol;

  if(argc != 4)
    sysError("invalid number of arguments to primitive", "PRIM_sockectConnect");

  sockfd = getInteger(args[0]);
  addr = charPtr(args[1]);
  servicename = charPtr(args[2]);
  protocol = charPtr(args[3]);

  /* Resolve the host name */
  if (!(hostaddr = gethostbyname(addr))) 
  {
    //*errcode = h_errno;
    return booleanSyms[booleanFalse];
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
    //*errcode = errno;
    return booleanSyms[booleanFalse];
  }

  return booleanSyms[booleanTrue];
}

object PRIM_socketBind(int primitiveNumber, object* args, int argc)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;

  int sockfd;
  const char *servicename, *protocol;

  if(argc != 3)
    sysError("invalid number of arguments to primitive", "PRIM_sockectBind");

  sockfd = getInteger(args[0]);
  servicename = charPtr(args[1]);
  protocol = charPtr(args[2]);

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;

  /* setup the servent struct using getservbyname */
  servaddr = getservbyname(servicename, protocol);
  socketaddr.sin_port = servaddr->s_port;

  memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

  /* everything is setup, now we connect */
  if(bind(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) 
    return newInteger(errno);

  return newInteger(0);
}


object PRIM_socketBindToPort(int primitiveNumber, object* args, int argc)
{
  struct sockaddr_in socketaddr;
  struct hostent *hostaddr;
  struct servent *servaddr;
  int result;
  int sockfd, port;

  if(argc != 2)
    sysError("invalid number of arguments to primitive", "PRIM_sockectBindToPort");

  sockfd = getInteger(args[0]);
  port = getInteger(args[1]);

  /* clear and initialize socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_addr.s_addr = INADDR_ANY;
  socketaddr.sin_port = htons(port);

  /* everything is setup, now we connect */
  if(bind(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) 
    return newInteger(errno);

  return newInteger(0);
}


object PRIM_socketSend(int primitiveNumber, object* args, int argc)
{
  size_t ret;

  int sockfd, len, flags;
  const char *data;

  if(argc != 3)
    sysError("invalid number of arguments to primitive", "PRIM_sockectSend");

  sockfd = getInteger(args[0]);
  data = charPtr(args[1]);
  flags = getInteger(args[2]);

  len = strlen(data);

  /* send our get request */
  if((ret = send(sockfd, data, len, flags)) < 0)
  {
    //*errcode = errno;
    return booleanSyms[booleanFalse];
  }

  return booleanSyms[booleanTrue];
}

object PRIM_socketListen(int primitiveNumber, object* args, int argc)
{
  int sockfd, backlog;

  if(argc != 2)
    sysError("invalid number of arguments to primitive", "PRIM_sockectListen");

  sockfd = getInteger(args[0]);
  backlog = getInteger(args[1]);

  if(listen(sockfd, backlog) < 0)
  {
    //*errcode = errno;
    return booleanSyms[booleanFalse];
  }

  return booleanSyms[booleanTrue];
}

object PRIM_socketAccept(int primitiveNumber, object* args, int argc)
{
  struct sockaddr_in socketaddr;
  int newsockfd;

  int sockfd;

  if(argc != 1)
    sysError("invalid number of arguments to primitive", "PRIM_sockectAccept");

  sockfd = getInteger(args[0]);

#ifndef WIN32
  socklen_t client_len;
#else
  int client_len;
#endif

  /* clear and initialize client socketaddr */
  memset(&socketaddr, 0, sizeof(socketaddr));

  newsockfd = accept(sockfd, (struct sockaddr*)&socketaddr, &client_len);

  //if(newsockfd < 0)
  //  *errcode = errno;

  return newInteger(newsockfd);
}


object PRIM_socketClose(int primitiveNumber, object* args, int argc)
{
  int sockfd, res;

  if(argc != 1)
    sysError("invalid number of arguments to primitive", "PRIM_sockectClose");

  sockfd = getInteger(args[0]);

#ifndef WIN32
  res = close(sockfd);
#else
  res = closesocket(sockfd);
#endif

  if(res == -1)
    return newInteger(errno);
  else
    return newInteger(0);
}


object PRIM_socketRead(int primitiveNumber, object* args, int argc)
{
  size_t res;
  int sockfd, len;
  char* buffer;
  object data;

  if(argc != 2)
    sysError("invalid number of arguments to primitive", "PRIM_sockectRead");

  sockfd = getInteger(args[0]);
  len = getInteger(args[1]);

  buffer = calloc(len + 1, sizeof(char));

  if((res = read(sockfd, buffer, len)) <= 0)
    return nilobj;

  data = newStString(buffer);

  free(buffer);

  return data;
}

int eagain()
{
#ifndef WIN32
  return EAGAIN;
#else
  return WSAEWOULDBLOCK;
#endif
}
  
const char* socketsPrimId = "6ab63be0-4c3d-11e2-bcfd-0800200c9a66";
PrimitiveTableEntry socketsPrims[] =
{
  { "socketsNew", PRIM_socketNew },
  { "socketsShutdown", PRIM_socketShutdown },
  { "socketsConnect", PRIM_socketConnect },
  { "socketsBind", PRIM_socketBind },
  { "socketsBindToPort", PRIM_socketBindToPort },
  { "socketsSend", PRIM_socketSend },
  { "socketsListen", PRIM_socketListen },
  { "socketsAccept", PRIM_socketAccept },
  { "socketsClose", PRIM_socketClose },
  { "socketsRead", PRIM_socketRead },
  { NULL, NULL }
};

void initialiseSocketsPrims()
{
  addPrimitiveTable("sockets", stringToUUID(socketsPrimId), socketsPrims);
}
