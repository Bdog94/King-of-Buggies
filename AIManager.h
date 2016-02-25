#ifndef AIMANAGER_H
#define AIMANAGER_H

#include "GameState.h"
#include "LevelInfo.h"
#include "Input.h"
#include "PxPhysicsAPI.h"
#include <iostream>

using namespace physx;

class AIManager {
public:

	int updateCounter = 0;
	PxTransform lastOurTransform;
	PxTransform lastBallTransform;
	PxVec3 rotationalTarget;
	vector<Input> inputs;
	Input updateAI(GameState* state);
	Input testAIEvade(GameState state);
	void initAI(int pNum);
	int getPlayerNum() {
		return playerNum;
	};

private:
	int playerNum;
	float facing(Entity* object, Entity* target);
	float beside(Entity* object, Entity* target);
	float lastDot = NULL;
	bool lastTurnL = false;
	float frameCounter = 0;
	GameState lastState;
};

#endif // AIMANAGER_H