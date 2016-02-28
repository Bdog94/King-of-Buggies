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
	Input testAIChase(GameState state);
	Input testAIEvade(GameState state, int playerNum);
	void initAI(int pNum);
	void initAI(GameState state);

private:
	vector<int> playerNums;
	vec3 prevPosition;
	float facing(Entity* object, Entity* target);
	float beside(Entity* object, Entity* target);
	float lastDot = NULL;
	bool lastTurnL = false;
	float frameCounter = 0;
	GameState lastState;
	bool reversing = false;
	float carSpeed = 0.3f;
};

#endif // AIMANAGER_H