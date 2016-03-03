#ifndef GAMESTATE_CPP
#define GAMESTATE_CPP

#include "GameState.h"

GameState::GameState()
{

}

void GameState::addPlayer(const PlayerInfo& pinfo) { players.push_back(pinfo); }
void GameState::addAI(const PlayerInfo& pinfo){ ai.push_back(pinfo); }
void GameState::setPlayer(unsigned int playerNum, const PlayerInfo& pinfo)
{
	if (playerNum < NUM_PLAYERS)
		players[playerNum] = pinfo;
	else
		printf("PlayerInfo>> Player index out of range\n");
}

PlayerInfo* GameState::getPlayer(unsigned int playerNum)
{
	if (playerNum < players.size())
		return &players[playerNum];
	else
		return NULL;
}

PlayerInfo* GameState::getAI(unsigned int aiNum)
{
	if (aiNum < ai.size())
		return &ai[aiNum];
	else
		return NULL;
}

void GameState::setGoldenBuggie(unsigned int playerNum)
{
	for (unsigned int i = 0; i < numberOfPlayers(); i++)
	{
		getPlayer(i)->setGoldenBuggie(false);
	}
	getPlayer(playerNum)->setGoldenBuggie(true);
	goldenBuggy = playerNum;
}

PlayerInfo* GameState::getGoldenBuggy()
{
	return &players[goldenBuggy];
}

PlayerInfo* GameState::getGoldenBuggie()
{
	for (int i = 0; i < players.size(); i++)
	{
		PlayerInfo player = players[i];
		if (player.isGoldenBuggie()){
			return &player;
		}

	}
	return NULL;
}

unsigned int GameState::numberOfPlayers(){ return players.size(); }
unsigned int GameState::numberOfAIs() { return ai.size(); }

void GameState::addCoin(const Coin& coin) { coins.push_back(coin); }

void GameState::setCoin(unsigned int coinNum, const Coin& coin)
{
	if (coinNum < coins.size())
		coins[coinNum] = coin;
	else
		printf("Coin>> Coin index out of range\n");
}

Coin* GameState::getCoin(unsigned int coinNum)
{
	if (coinNum < coins.size())
		return &coins[coinNum];
	else
		return NULL;
}

unsigned int GameState::numberOfCoins(){ return coins.size(); }

void GameState::addPowerup(const Powerup& powerup) { powerups.push_back(powerup); }

void GameState::setPowerup(unsigned int powerupNum, const Powerup& powerup)
{
	if (powerupNum < powerups.size())
		powerups[powerupNum] = powerup;
	else
		printf("Powerup>> Powerup index out of range\n");
}

Powerup* GameState::getPowerup(unsigned int powerupNum)
{
	if (powerupNum < powerups.size())
		return &powerups[powerupNum];
	else
		return NULL;
}

unsigned int GameState::numberOfPowerups(){ return powerups.size(); }

void GameState::addPowerupBox(const PowerupBox& box) { boxes.push_back(box); }

void GameState::setPowerupBox(unsigned int boxNum, const PowerupBox& box)
{
	if (boxNum < boxes.size())
		boxes[boxNum] = box;
	else
		printf("PowerupBox>> PowerupBox index out of range\n");
}

PowerupBox* GameState::getPowerupBox(unsigned int boxNum)
{
	if (boxNum < boxes.size())
		return &boxes[boxNum];
	else
		return NULL;
}

unsigned int GameState::numberOfPowerupBoxes(){ return boxes.size(); }

void GameState::checkCoinCollision(vec3 playerPos) {
	for (int i = 0; i < coins.size(); i++) {
		vec3 pos = coins[i].getPos();
		
		
		if (coins[i].isCollided()) {
			// countdown to coin respawn
			coins[i].decrementCountdown();
			if (coins[i].getCountdown() < 0) {
				// reset coin to original spawn location
				vec3 newPos = pos;
				newPos.y = pos.y + 4;
				coins[i].setPos(newPos);
				coins[i].setCollided(false);
			}
		}
		else {
			vec3 difference = pos - playerPos;

			//change coin to spawn below level if it is hit by a player
			if ((abs(difference.x) < 1.4) && (abs(difference.y) < 1.4) && (abs(difference.z) < 1.4)) {
				vec3 newPos = pos;
				newPos.y = pos.y - 4;
				coins[i].setPos(newPos);
				coins[i].setCollided(true);
				coins[i].startCountdown();
			}
		}
	}
}

#endif // GAMESTATE_CPP