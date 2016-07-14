#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <thread>
#include <mutex>

#include "Connection.h"

using namespace std;

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")


mutex mu;

bool running = true;
void listenGame(SOCKET&);

struct GameInfo {
	string type;
	string map;
	int difficulty;
};

void listenGame(SOCKET& ConnectSocket) {
	int ret;
	char buf[DEFAULT_BUFLEN];

	//Used to test if the client has been pinged
	bool pinged;
	int timer = 0;
	bool error = false;

	while (running) {
		timer++;
		//Recieve message from client and save to buf variable.
		ret = recv(ConnectSocket, buf, sizeof(buf), 0);

		//Check for errors 
		if (ret == 0 && error == false) {
			cout << "Error: Problem Recieving Message" << endl;
			error = true;
			strcpy_s(buf, " ");
		}

		//Recieve welcome message
		if (strstr(buf, "welcome")) {
			cout << "Welcome To Server" << endl;
			strcpy_s(buf, " ");
		}

		if (strstr(buf, "spectator")) {
			cout << "Welcome To Spectators" << endl;
			strcpy_s(buf, " ");
		}

		if (strstr(buf, "ping")) {
			send(ConnectSocket, "pong", (int)strlen("specQuit"), 0);
			pinged = true;
			cout << "PONG" << endl;
			strcpy_s(buf, " ");
		}

		if (strstr(buf, "map")) {
			cout << "Welcome To Game" << endl;
		}

		if (timer >= 6000) {
			if (!pinged) {
				cout << "Error: Server Has Disconected" << endl;
				cout << "Enter <e> To Exit Program" << endl;
				running = false;
				ExitThread(0);
			}
			else {
				pinged = false;
				timer = 0;
				cout << "Reset Timer And Pinged" << endl;
			}
		}

		Sleep(1);
	}
}

int main() {

	// Getting system information to change colour of text in command prompt.
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	//Variables
	Connection c;
	int iResult;

	thread t1(listenGame, std::ref(c.ConnectSocket));
	t1.detach();

	if (c.failedConnect) {
		mu.lock();
		cout << "Failed To Init" << endl;
		mu.unlock();
		return 0;
	}


	while (running) {
		int input = 0;
		cin >> input;
		cout << endl;
		if (input == 9) {
			cout << "QUITT GAME!!" << endl;
			iResult = send(c.ConnectSocket, "exit", (int)strlen("exit"), 0);
			running = false;
			Sleep(100);
			closesocket(c.ConnectSocket);
			WSACleanup();

			return 0;
		}
	}

	cout << "QUITT GAME!!" << endl;
	closesocket(c.ConnectSocket);
	WSACleanup();

	return 0;

}
