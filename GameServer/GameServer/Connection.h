#include <winsock2.h>
#include <stdlib.h>
#include <ws2tcpip.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class Connection {

public:

	Connection(void);

	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
};