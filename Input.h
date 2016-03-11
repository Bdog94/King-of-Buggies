#ifndef INPUT_H
#define INPUT_H

struct Input
{
	float forward;
	float backward;
	float turnL;
	float turnR;
	float tiltForward;
	float tiltBackward;
	float camV;
	float camH;

	bool drift;
	bool powerup;
	bool menu;
	bool jump;
	bool horn;

	bool cheat_coin;
	bool isKeyboard;
};

#endif //INPUT_H