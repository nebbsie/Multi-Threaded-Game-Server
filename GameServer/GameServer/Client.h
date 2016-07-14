#include <winsock2.h>

class Client {

private:
	SOCKET sock;
	bool connected;
	
public:
	
	void setSocket(SOCKET sockIn);
	SOCKET getSocket();
	void setConnected(bool connectedIn);
	bool getConnected();
	bool terminate;
	std::thread::id pid;
};