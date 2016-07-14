#include "Game.h"
#include <cstdlib>
#include <sys/timeb.h>
using namespace std;

Game::Game() {
	inGameCount = 0;
	spectatorsCount = 0;
	serverRunning = true;
	startTime = GetTickCount();
}

string Game::getType(){
	return type;
}

string Game::getMap(){
	return map;
}

int Game::getDifficulty(){
	return difficulty;
}

int Game::getStartTime() {
	return startTime;
}

int Game::getmaxConnections(){
	return maxConnections;
}

int Game::getminConnections(){
	return minConnections;
}

int Game::getInGameCount(){
	return inGameCount;
}

int Game::getSpectators(){
	return spectatorsCount;
}

void Game::incrementSpectatosCount(){
	totalConnectionsCount++;
	totalSpectatorsCount++;
	spectatorsCount++;
}

void Game::decrementSpectators(){
	spectatorsCount--;
}

void Game::incrementTotalClientsLeftCount() {
	totalClientsLeftCount++;
}

void Game::incrementInGameClientsCount(){
	totalConnectionsCount++;
	inGameCount++;
}

void Game::decrementInGameClientsCount(){
	inGameCount--;
}

int Game::getTotalConnectionsCount() {
	return totalConnectionsCount;
}

int Game::getTotalClientsLeftCount() {
	return totalClientsLeftCount;
}

bool Game::getServerRunning(){
	return serverRunning;
}

void Game::setServerRunning(bool boolean) {
	serverRunning = boolean;
}

int Game::getTotalSpectatorsCount() {
	return totalSpectatorsCount;
}

void Game::populate(string typein, string mapin, int difficultyin, int minConnectionsin, int maxConnectionsin) {
	type = typein;
	map = mapin;
	difficulty = difficultyin;
	minConnections = minConnectionsin;
	maxConnections = maxConnectionsin;
}

int Game::getPositionInGame(thread::id pid){
	for (unsigned int i = 0; i < clients.size(); i++) {
		if (clients[i].pid == pid) {
			return clients[i].pos;
		}
	}
	return NULL;
}

int Game::getPositionInSpectators(thread::id pid){
	for (int i = 0; i < spectators.size(); i++) {
		if (spectators[i].pid == pid) {
			return spectators[i].pos;
		}
	}
	return NULL;
}