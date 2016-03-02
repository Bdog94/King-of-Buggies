#ifndef INPUTSMOOTHER_H
#define INPUTSMOOTHER_H

#include "Input.h"

class InputSmoother
{
public:
	InputSmoother();
	~InputSmoother();
	Input smooth(Input in);

private:
	float lastForward = 0;
	float lastBackward = 0;
	float lastTurnL = 0;
	float lastTurnR = 0;
	// Set higher to have faster changes
	// Lower to have smoother changes
	float accelStep = 0.08;
	float turnStep = 0.08;

	bool menuLock = false;
	int menuFrameCounter = 0;
	int menuCounterMax = 5;

	bool powerupLock = false;
	int powerupFrameCounter = 0;
	int powerupCounterMax = 5;

	bool driftLock = false;
	int driftFrameCounter = 0;
	int driftCounterMax = 5;
};

#endif // INPUTSMOOTHER_H

