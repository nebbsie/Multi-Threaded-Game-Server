#include <string>
#include <thread>
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <deque>
#include <time.h>
using namespace std;

struct Client {
	SOCKET sock;
	thread::id pid;
	int pos;
	int thread;
	bool connected;
	bool terminate;
	bool spectator;
};

class Game{
	public:
		//Constructor
		Game();

		//Getters
		string getType();
		string getMap();
	
		int getDifficulty();
		int getmaxConnections();
		int getminConnections();
		int getInGameCount();
		int getSpectators();
		int getTotalConnectionsCount();
		int getTotalSpectatorsCount();
		int getTotalClientsLeftCount();
		int getStartTime();
		bool getServerRunning();
		void setServerRunning(bool boolean);

		//Incrementers and decrementers
		void incrementSpectatosCount();
		void incrementInGameClientsCount();
		void decrementSpectators();
		void decrementInGameClientsCount();
		void incrementTotalClientsLeftCount();
	
		//Set Game Info
		void populate(string, string, int, int, int);
		int getPositionInGame(thread::id);
		int getPositionInSpectators(thread::id);
	
		//Vectors that hold clients and spectators seperatly
		vector<Client> clients;
		vector<Client> spectators;

	private:
		//Game settings
		int difficulty;
		int maxConnections;
		int minConnections;
		string type;
		string map;

		//Server stats
		long startTime;
		long endTIme;

		//Server info
		int inGameCount;
		int spectatorsCount;
		int totalConnectionsCount;
		int totalSpectatorsCount;
		int totalClientsLeftCount;
		bool serverRunning;
};