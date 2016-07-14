#include "Client.h"

using namespace std;

bool Client::getConnected() {
	return connected;
}

void Client::setConnected(bool connectedIn) {
	connected = connectedIn;
}

SOCKET Client::getSocket() {
	return sock;
}

void Client::setSocket(SOCKET sockIn) {
	sock = sockIn;
}