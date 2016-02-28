#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <glm/glm.hpp>
#include "Entity.h"

class PlayerInfo: public Entity {
public:
	PlayerInfo();
	PlayerInfo(unsigned int renderID, unsigned int physicsID, unsigned int* wheelRenderIDs);
	bool isGoldenBuggie();
	bool isAI();

	unsigned int getWheelRenderID(unsigned int wheelNum);
	mat4 getWheelTransform(unsigned int wheelNum);
	void setWheelTransform(unsigned int wheelNum, const mat4& transform);

	void setFSpeed(float speed) { fSpeed = speed; };
	float getFSpeed() { return fSpeed; };

protected:
	unsigned int wheelRenderIDs[4];
	mat4 wheelTransforms [4];

	int powerup;
	int numCoins;
	int powerupCounter;
	bool goldenBuggie;
	bool ai;
	float timeGolden;
	float fSpeed;
};

#endif // PLAYERINFO_H