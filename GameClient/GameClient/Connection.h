#include <winsock2.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class Connection {

public:

	Connection();

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL;
	struct addrinfo hints;
	int iSendResult;
	char *sendbuf = "hello";
	int iResult;
	bool failedConnect;


private:

};