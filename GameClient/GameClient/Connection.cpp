#include "Connection.h"
#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <string>
#include <sstream>

Connection::Connection()
{

	std::ifstream infile("clientsetup.txt");
	std::string line;
	std::string IP;
	int count = 1;

	while (getline(infile, line)) {
		IP = line;
		std::cout << "Connecting To: " << IP << std::endl;
	}


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		failedConnect = true;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(IP.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		failedConnect = true;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		//Create a socket to connect to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error");
			WSACleanup();
			failedConnect = true;
		}

		//Connect to server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	//Check if connected to server
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to a server!\n");
		system("pause");
		WSACleanup();
		failedConnect = true;
	}

	//Send initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("Send failed\n");
		closesocket(ConnectSocket);
		WSACleanup();
		failedConnect = true;
	}

}