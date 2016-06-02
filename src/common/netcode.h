
#ifndef _EQP_NETCODE_H_
#define _EQP_NETCODE_H_

#ifdef EQP_WINDOWS
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
#else
# include <errno.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netdb.h>
# include <signal.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netinet/tcp.h>
# include <net/if.h>
# include <endian.h>
#endif

#define toNetworkUint16 htons
#define toNetworkUint32 htonl
#define toHostUint16 ntohs
#define toHostUint32 ntohl

#ifdef EQP_WINDOWS
# define toNetworkUint64 htonll
# define toHostUint64 ntohll
#else
# define toNetworkUint64 htobe64
# define toHostUint64 be64toh
#endif

#ifndef EQP_WINDOWS
# define closesocket close
# define INVALID_SOCKET -1
#else
typedef int socklen_t;
#endif

typedef struct sockaddr_in IpAddress;

#endif//_EQP_NETCODE_H_
