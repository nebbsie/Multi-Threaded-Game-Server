#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <thread>
#include <Windows.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <mutex>
#include <ctime>

#include "Connection.h"
#include "Game.h"

//Colours for the screen
#define BLUE 9
#define GREEN 10
#define RED 12
#define YELLOW 14
#define GAME 15
#define PING 13

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")



// Getting system information to change colour of text in command prompt.
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

std::mutex mu;



using namespace std;

//Prototypes
void spectate(Game&, SOCKET);
void checkArray(Game&);
void receieve_client(Game&, SOCKET);
void gameupdateThread(Game&);
void WriteLogFile(const char* szString, thread::id);
void WriteLogFileEnd(Game&);
void userInput(Game&, Connection&);
void endGame(Game&, Connection&);
void gameRun(Game&);

void spectate(Game& game, SOCKET sock) {
	//Variables
	char buf[100];
	int ret = 0;
	int pos = game.getSpectators();

	Client c;
	c.sock = sock;
	c.pid = std::this_thread::get_id();
	c.connected = true;
	c.terminate = false;
	c.spectator = true;
	c.pos = pos;

	//Add client to spectator list and increment spectators
	game.spectators.push_back(c);
	game.incrementSpectatosCount();

	//Reshuffle the spectators vecotr
	for (unsigned int i = 0; i < game.getSpectators(); i++) {
		game.spectators[i].pos = i;
	}

	//Announce to server that user has been added
	SetConsoleTextAttribute(h, BLUE);
	std::cout << "Added To Spectate Mode PID: " << game.spectators[game.getPositionInSpectators(this_thread::get_id())].pid << std::endl;

	//Send client infotmation to show that they have joined and check for errors
	strcpy_s(buf, "spectator");
	ret = send(game.spectators[game.getPositionInSpectators(this_thread::get_id())].sock, buf, sizeof(buf), 0);
	if (ret == 0) {
		SetConsoleTextAttribute(h, RED);
		cout << "PROBLEM WITH SEND" << endl;
	}

	//Used to send ping
	int timer = 0;
	bool error = false;

	//Main thread loop
	while (true) {
		timer++;

		//Recieve message from client and save to buf variable
		ret = recv(game.spectators[game.getPositionInSpectators(this_thread::get_id())].sock, buf, sizeof(buf), 0);

		//Check for errors
		if (ret == 0) {
			SetConsoleTextAttribute(h, RED);
			cout << "PROBLEM WITH RECV" << endl;
		}

		//Check if Client thread has been shutdown form changing to game group
		if (game.spectators[game.getPositionInSpectators(this_thread::get_id())].terminate) {
			game.spectators.erase(game.spectators.begin() + game.getPositionInSpectators(this_thread::get_id()));
			game.decrementSpectators();

			//Reshuffle the spectators vecotr
			for (unsigned int i = 0; i < game.spectators.size(); i++) {
				game.spectators[i].pos = i;
			}

			//Exit thread
			ExitThread(0);
		}

		//Look For a pong
		if (strstr(buf, "pong")) {
			SetConsoleTextAttribute(h, PING);
			cout << "PONG Recieved: " << this_thread::get_id() << endl;
			WriteLogFile("Pong Recieved", this_thread::get_id());
			error = false;
			strcpy(buf, " ");
		}

		//Check if client has requested to quit
		if (strstr(buf, "exit")) {

			SetConsoleTextAttribute(h, RED);
			cout << "Spectator: " << game.spectators[game.getPositionInSpectators(this_thread::get_id())].pid << " Exited From Game." << endl;
			closesocket(game.spectators[game.getPositionInSpectators(this_thread::get_id())].sock);

			//Try to erase player and reshuffle vector
			try {
				mu.lock();
				game.spectators.erase(game.spectators.begin() + game.getPositionInSpectators(this_thread::get_id()));
				game.decrementSpectators();
				game.incrementTotalClientsLeftCount();
				mu.unlock();
				try {
					mu.lock();
					for (unsigned int i = 0; i < game.spectators.size(); i++) {
						game.spectators[i].pos = i;
					}
					mu.unlock();
				}
				catch (const std::exception&) {
					cout << "FAILED TO REPOSITION" << endl;
				}
				ExitThread(0);
			}
			catch (const std::exception&) {
				cout << "FAILED TO DELETE" << endl;
			}
		}

		//Check ping pong
		if (timer >= 5000) {
			if (!error) {
				SetConsoleTextAttribute(h, PING);
				cout << "PING Sent: " << this_thread::get_id() << endl;
				WriteLogFile("PING Transmitted", this_thread::get_id());
				SetConsoleTextAttribute(h, GREEN);
				strcpy_s(buf, "ping");
				ret = send(game.spectators[game.getPositionInSpectators(this_thread::get_id())].sock, buf, sizeof(buf), 0);
				timer = 0;
				error = true;
			}
			else {
				SetConsoleTextAttribute(h, RED);
				cout << "Spectator: " << game.spectators[game.getPositionInSpectators(this_thread::get_id())].pid << " Lost Connection." << endl;
				WriteLogFile("Spectator lost connection to server", this_thread::get_id());
				closesocket(game.spectators[game.getPositionInSpectators(this_thread::get_id())].sock);

				//Try to erase player and reshuffle vector
				try {
					mu.lock();
					game.spectators.erase(game.spectators.begin() + game.getPositionInSpectators(this_thread::get_id()));
					game.decrementSpectators();
					game.incrementTotalClientsLeftCount();
					mu.unlock();
					try {
						mu.lock();
						for (unsigned int i = 0; i < game.spectators.size(); i++) {
							game.spectators[i].pos = i;
						}
						mu.unlock();
					}
					catch (const std::exception&) {
						cout << "Error: Failed To Reposition" << endl;
					}
					ExitThread(0);
				}
				catch (const std::exception&) {
					cout << "Error: Failed To Delete" << endl;
				}
			}
		}

		if (!game.getServerRunning()) {
			ExitThread(0);
		}

		Sleep(1);
	}
}

void checkArray(Game& game) {

	/*Check the array for players and print out count
	of the eachh in game and in spectators to the screen */

	int count = 0;

	while (true) {

		for (unsigned int i = 0; i < game.clients.size(); i++) {
			if (game.clients[i].connected) {
				count++;
			}
		}

		for (unsigned int i = 0; i < game.spectators.size(); i++) {
			if (game.spectators[i].connected) {
				count++;
			}
		}

		if (game.spectators.size() > 0) {
			cout << "Next To Play: " << game.spectators[0].pid << endl;
		}

		if (!game.getServerRunning()) {
			ExitThread(0);
		}

		SetConsoleTextAttribute(h, YELLOW);
		cout << "Connected Clients: " << count << " In Game: " << game.getInGameCount() << " Spectating: " << game.getSpectators() << endl;
		SetConsoleTextAttribute(h, GREEN);
		count = 0;

		Sleep(4000);
	}

}

void recieve_client(Game& game, SOCKET sock) {

	//Get position to place into vector  & create variables
	int pos = game.getInGameCount();
	char buf[100];
	int ret = 0;

	//Create a client to add the the game vector
	Client c;
	c.sock = sock;
	c.pid = std::this_thread::get_id();
	c.connected = true;
	c.terminate = false;
	c.spectator = false;
	c.pos = pos;

	//Add to back of clients and increment client count
	game.clients.push_back(c);
	game.incrementInGameClientsCount();

	//Print the thread number and the id of the thread
	
	SetConsoleTextAttribute(h, BLUE);
	mu.lock();
	cout << "Thread: " << game.getInGameCount() << " Created. PID: " << this_thread::get_id() << endl;
	mu.unlock();
	SetConsoleTextAttribute(h, GREEN);
	

	//Reposition clients
	for (unsigned int i = 0; i < game.clients.size(); i++) {
		game.clients[i].pos = i;
	}

	//Write to log file
	mu.lock();
	WriteLogFile("Client joined game", this_thread::get_id());
	mu.unlock();

	//Send welcome message to client
	strcpy_s(buf, "welcome");
	ret = send(game.clients[game.getPositionInGame(this_thread::get_id())].sock, buf, sizeof(buf), 0);


	//If the send failed print message
	if (ret == 0) {
		SetConsoleTextAttribute(h, RED);
		cout << "Error: Problem Sending Welcome Message" << endl;
	}

	int timer = 0;
	bool error = false;
	char lastEntry[100];

	//Main thread loop
	while (true) {
		timer++;

		//Recieve message from client and save to buf variable.
		ret = recv(game.clients[game.getPositionInGame(this_thread::get_id())].sock, buf, sizeof(buf), 0);

		//Check if the recieved message has an error
		if (ret == 0) {
			SetConsoleTextAttribute(h, RED);
			cout << "Error: Problem Recieving Message" << endl;
		}

		if (strstr(buf, "pong")) {
			SetConsoleTextAttribute(h, PING);
			cout << "PONG Recieved: " << this_thread::get_id() << endl;
			WriteLogFile("Pong Recieved", this_thread::get_id());
			error = false;
			strcpy(buf, " ");
		}

		//Check if the user has requested to quit
		else if (strstr(buf, "exit")) {
			SetConsoleTextAttribute(h, RED);
			cout << "Client: " << game.clients[game.getPositionInGame(this_thread::get_id())].pid << " Exited From Game." << endl;
			WriteLogFile("Client left game", this_thread::get_id());
			closesocket(game.clients[game.getPositionInGame(this_thread::get_id())].sock);

			//Try to erase player and reshuffle vector
			try {
				mu.lock();
				game.clients.erase(game.clients.begin() + game.getPositionInGame(this_thread::get_id()));
				game.decrementInGameClientsCount();
				game.incrementTotalClientsLeftCount();
				mu.unlock();
				try {
					mu.lock();
					for (unsigned int i = 0; i < game.clients.size(); i++) {
						game.clients[i].pos = i;
					}
					mu.unlock();
				}
				catch (const std::exception&) {
					cout << "Error: Failed To Reposition" << endl;
				}
				ExitThread(0);
			}
			catch (const std::exception&) {
				cout << "Error: Failed To Delete" << endl;
			}
		}

		if (timer >= 5000) {
			if (!error) {
				SetConsoleTextAttribute(h, PING);
				cout << "PING Sent: " << this_thread::get_id() << endl;
				WriteLogFile("PING Transmitted", this_thread::get_id());
				SetConsoleTextAttribute(h, GREEN);
				strcpy_s(buf, "ping");
				ret = send(game.clients[game.getPositionInGame(this_thread::get_id())].sock, buf, sizeof(buf), 0);
				timer = 0;
				error = true;
			}
			else {
				SetConsoleTextAttribute(h, RED);
				cout << "Client: " << game.clients[game.getPositionInGame(this_thread::get_id())].pid << " Lost Connection." << endl;
				WriteLogFile("Client lost connection to server", this_thread::get_id());
				closesocket(game.clients[game.getPositionInGame(this_thread::get_id())].sock);

				//Try to erase player and reshuffle vector
				try {
					mu.lock();
					game.clients.erase(game.clients.begin() + game.getPositionInGame(this_thread::get_id()));
					game.decrementInGameClientsCount();
					game.incrementTotalClientsLeftCount();
					mu.unlock();
					try {
						mu.lock();
						for (unsigned int i = 0; i < game.clients.size(); i++) {
							game.clients[i].pos = i;
						}
						mu.unlock();
					}
					catch (const std::exception&) {
						cout << "Error: Failed To Reposition" << endl;
					}
					ExitThread(0);
				}
				catch (const std::exception&) {
					cout << "Error: Failed To Delete" << endl;
				}
			}
		}

		if (!game.getServerRunning()) {
			ExitThread(0);
		}

		Sleep(1);
	}
}

void gameupdateThread(Game& game) {

	while (true) {

		if (!game.getServerRunning()) {
			ExitThread(0);
		}

		if (game.getInGameCount() < game.getmaxConnections()) {
			if (game.getSpectators() > 0) {

				Client cl;
				game.spectators[0].terminate = true;
				game.spectators[0].connected = false;
				cl = game.spectators[0];
				cout << "Client: " << cl.pid << " has been moved to game." << endl;
				std::thread t4(recieve_client, std::ref(game), cl.sock);
				t4.detach();

			}
		}
	}
}

void WriteLogFile(const char* string, thread::id PID) {

	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	int hour = timeinfo->tm_hour;
	int min = timeinfo->tm_min;
	int sec = timeinfo->tm_sec;
	int day = timeinfo->tm_mday;
	int mon = timeinfo->tm_mon + 1;
	int yr = timeinfo->tm_year;

	yr = yr + 1900;

	FILE* pFile = fopen("log.txt", "a");
	fprintf(pFile, "%i/%i/%i  %i:%i:%i: <%i> %s \n", day, mon, yr, hour, min, sec, PID, string);
	fclose(pFile);
}

void WriteLogFileEnd(Game& game) {

	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);


	int hour = timeinfo->tm_hour;
	int min = timeinfo->tm_min;
	int sec = timeinfo->tm_sec;
	int day = timeinfo->tm_wday;
	int mon = timeinfo->tm_mon;
	int yr = timeinfo->tm_year;
	yr = yr + 1900;

	long endTime = GetTickCount();
	long totalTime = endTime - game.getStartTime();
	totalTime = totalTime / 1000;

	FILE* pFile = fopen("log.txt", "a");
	fprintf(pFile, "\nTotal Clients Joined: %i\n", game.getTotalConnectionsCount());
	fprintf(pFile, "Total Spectators Joined: %i\n", game.getTotalSpectatorsCount());
	fprintf(pFile, "Total Clients Left: %i\n", game.getTotalClientsLeftCount());
	fprintf(pFile, "Total Server Running Time: %i\n", totalTime);
	fprintf(pFile, "-------------------------------\n\n");
	fclose(pFile);

}

void endServer(Game& game, Connection& c) {
	WriteLogFileEnd(game);
	game.setServerRunning(false);
	closesocket(c.ClientSocket);
	WSACleanup();
}

void userInput(Game& game, Connection& c) {

	int input;
	cin >> input;

	while (input != 9) {
		cin >> input;
	}

	if (input == 9) {
		cout << "Ending Server..." << endl;
		endServer(std::ref(game), std::ref(c));

	}
}

void gameRun(Game& game) {
	while (game.getServerRunning()) {
		if (game.getInGameCount() >= game.getminConnections()) {
			mu.lock();
			SetConsoleTextAttribute(h, GAME);
			cout << "Game Running" << endl;
			mu.unlock();
			Sleep(5000);
		}
		else {
			Sleep(5000);
			mu.lock();
			SetConsoleTextAttribute(h, GAME);
			cout << "Game Not Running" << endl;
			mu.unlock();
			
		}

	}

	ExitThread(0);
}

int main() {

	//Variables
	Game game;
	string type;
	string map;
	int difficulty;
	int minConnections;
	int maxConnections;

	Connection c = Connection();

	//Read server config files
	ifstream infile("serverconfig.txt");
	string line;
	int count = 1;

	while (getline(infile, line)){
		switch (count){
		case 1:
			type = line;
			break;
		case 2:
			map = line;
			break;
		case 3:
			difficulty = std::stoi(line);
			break;
		case 4:
			minConnections = std::stoi(line);
			break;
		case 5:
			maxConnections = std::stoi(line);
			break;
		default:
			break;
		}

		count++;
	}

	game.populate(type, map, difficulty, minConnections, maxConnections);

	//Recieve from user server information and print it out
	cout << "-------------------------------------------------" << endl;
	cout << "		   Game Server" << endl;
	cout << "-------------------------------------------------" << endl << endl;

	cout << "Game Mode: " << type << endl;
	cout << "Map: " << map << endl;
	cout << "Difficulty: " << difficulty << endl;
	cout << "Min Connections: " << minConnections << endl;
	cout << "Max Connections: " << maxConnections << endl << endl;

	cout << "-------------------------------------------------" << endl << endl << endl;


	sockaddr from;
	int fromlen = sizeof(from);


	thread t2(checkArray, ref(game));
	t2.detach();

	thread t3(gameupdateThread, ref(game));
	t3.detach();

	thread t5(gameRun, ref(game));
	t5.detach();

	thread t9(userInput, ref(game), ref(c));
	t9.detach();


	while (game.getServerRunning()) {

		c.ClientSocket = accept(c.ListenSocket, &from, &fromlen);

		u_long iMode = 1;
		ioctlsocket(c.ClientSocket, FIONBIO, &iMode);

		if ((game.getInGameCount() < game.getmaxConnections())) {
			if (!game.getSpectators() > 0 && game.getServerRunning()) {
				cout << "CREATE CLIENT" << endl;
				std::thread t1(recieve_client, std::ref(game), c.ClientSocket);
				t1.detach();
			}
		}
		else {
			std::thread t2(spectate, std::ref(game), c.ClientSocket);
			t2.detach();
		}
	}
	return 0;

}