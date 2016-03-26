#ifndef GAMEMANAGER_CPP
#define GAMEMANAGER_CPP

#include "GameManager.h"

GameManager::GameManager(GLFWwindow* newWindow) : renderer(newWindow), input(newWindow), state(), physics()
{
	for (unsigned int i = 0; i < MAX_PLAYERS; i++)
	{
		cam[i] = Camera(vec3(0.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 5.0), MODELVIEWER_CAMERA);
	}

	srand(time(NULL));
	window = newWindow;
	mat = Diffuse();
	shinyMat = Specular(20.f);
	tsMat = TorranceSparrow(10.f);
	matteMat = TorranceSparrow(0.5f);
	skyMaterial = Unshaded();

	timePassedDecoy = 0;

	renderer.loadPerspectiveTransform(0.1f, 100.f, 80.f);		//Near, far, fov
	renderer.loadCamera(&cam[0]);

	ai = AIManager(&state);


	for (int i = 0; i < 5; i++) {
		smoothers.push_back(InputSmoother());
	}

	//TODO: Put this indexing somewhere useful;
	ai.initAI(1);

	// setup interface according to window dimensions
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	_interface = InterfaceManager(width, height);
//	_interface.setWindowDim(width, height);

	playerColours.push_back(0);
	playerColours.push_back(1);
	playerColours.push_back(2);
	playerColours.push_back(3);
}


mat4 scaleMatrix(vec3 scale)
{
	mat4 scaleTransform(1.f);

	scaleTransform[0][0] = scale.x;
	scaleTransform[1][1] = scale.y;
	scaleTransform[2][2] = scale.z;

	return scaleTransform;
}

vec3 PxToVec3(PxVec3 vec) { return vec3(vec.x, vec.y, vec.z); }

void GameManager::cameraEnvironmentCollision(Camera* cam)
{
	cam->resetCameraDistance();

	vec3 origin = cam->getViewCenter();
	vec3 dir = cam->getPos() - origin;
	float maxDist = length(dir);
	dir = normalize(dir);

	float distance = physics.getEnvironmentRaycastResult(origin, dir, maxDist);
	if (distance > 0)
	{
		vec3 newPos = origin + dir*distance;
		cam->setPos(newPos);
	}
}

void GameManager::createPlayer(vec3 position, VehicleTraits traits, unsigned int texID)
{
	//Make argument to function later

	unsigned int chassisRenderID = renderer.generateObjectID();
	unsigned int physicsID = physics.vehicle_create(traits, position);

	vec3 colour = vehicleColours[state.numberOfPlayers()];

	MeshObject* playerMesh = meshInfo.getMeshPointer(BUGGY);
	renderer.assignMeshObject(chassisRenderID, playerMesh);
	renderer.assignMaterial(chassisRenderID, &tsMat);
	if (state.numberOfPlayers() == 0) {
		renderer.assignTexture(chassisRenderID, meshInfo.getGoldenBuggyTexID());
	}
	else { renderer.assignTexture(chassisRenderID, texID); }

	MeshObject* wheelMesh = meshInfo.getMeshPointer(WHEEL);
	unsigned int wheelIDs[4];

	
	for (unsigned int i = 0; i < 4; i++)
	{
		wheelIDs[i] = renderer.generateObjectID();
		renderer.assignMeshObject(wheelIDs[i], wheelMesh);
		renderer.assignMaterial(wheelIDs[i], &matteMat);
		renderer.assignScale(wheelIDs[i], 
			scaleMatrix(vec3(traits.wheelWidth, traits.wheelRadius, traits.wheelRadius)));
		renderer.assignColor(wheelIDs[i], vec3(0.2f, 0.2f, 0.2f));
	}

	state.addPlayer(PlayerInfo(chassisRenderID, physicsID, wheelIDs, colour, texID));
}

void GameManager::createDecoyGoldenBuggie(vec3 position, VehicleTraits traits)
{
	unsigned int chassisRenderID = renderer.generateObjectID();
	unsigned int physicsID = physics.vehicle_create(traits, position);

	vec3 colour = vehicleColours[state.numberOfPlayers()];
	unsigned int texID = meshInfo.getBuggyTexID(state.numberOfPlayers());

	MeshObject* playerMesh = meshInfo.getMeshPointer(BUGGY);
	renderer.assignMeshObject(chassisRenderID, playerMesh);
	renderer.assignMaterial(chassisRenderID, &tsMat);
	
	renderer.assignTexture(chassisRenderID, meshInfo.getGoldenBuggyTexID());
	

	MeshObject* wheelMesh = meshInfo.getMeshPointer(WHEEL);
	unsigned int wheelIDs[4];


	for (unsigned int i = 0; i < 4; i++)
	{
		wheelIDs[i] = renderer.generateObjectID();
		renderer.assignMeshObject(wheelIDs[i], wheelMesh);
		renderer.assignMaterial(wheelIDs[i], &matteMat);
		renderer.assignScale(wheelIDs[i],
			scaleMatrix(vec3(traits.wheelWidth, traits.wheelRadius, traits.wheelRadius)));
		renderer.assignColor(wheelIDs[i], vec3(0.2f, 0.2f, 0.2f));
	}


	//state.addPlayer(PlayerInfo(chassisRenderID, physicsID, wheelIDs, colour, texID));
	state.addDecoy(PlayerInfo(chassisRenderID, physicsID, wheelIDs, colour, texID ));
	smoothers.push_back(InputSmoother());

	
}

void GameManager::startDecoy(float time)
{
	timePassedDecoy += time;
}

bool GameManager::isDecoyTimerUp()
{
	if (timePassedDecoy > 50000)
	{
		timePassedDecoy = 0;
		return true;
	}
	return false;
}

void GameManager::createGroundPlane(vec3 normal, float offset)
{
	surfacePhysicsID = physics.ground_createPlane(vec3(0.f, 1.f, 0.f), 0.f);

	surfaceRenderID = renderer.generateObjectID();
	renderer.assignPlane(surfaceRenderID, 30.f, &surfaceVertices, &surfaceNormals, &surfaceIndices);
	renderer.assignMaterial(surfaceRenderID, &matteMat);
	renderer.assignColor(surfaceRenderID, vec3(0.5f, 0.5f, 0.5f));

}
void GameManager::createLevel(unsigned int objectID) {
	MeshObject* levelMesh = meshInfo.getMeshPointer(DONUTLEVEL);
	surfacePhysicsID = physics.ground_createGeneric(levelMesh);

	surfaceRenderID = renderer.generateObjectID();

	renderer.assignMeshObject(surfaceRenderID, levelMesh);
	renderer.assignMaterial(surfaceRenderID, &tsMat);
	renderer.assignColor(surfaceRenderID, vec3(0.5f, 0.5f, 0.5f));
	renderer.assignTexture(surfaceRenderID, levelMesh->getTextureID());

}

void GameManager::createBall(float radius)
{
	spherePhysicsID = physics.dynamic_createSphere(radius, vec3(0.f, 10.f, 0.f));

	sphereRenderID = renderer.generateObjectID();
	renderer.assignSphere(sphereRenderID, radius, 200, &ballVertices, &ballNormals, &ballUVs, &ballIndices);
	renderer.assignMaterial(sphereRenderID, &tsMat);
	renderer.assignTexture(sphereRenderID, 1);
	//renderer.assignColor(sphereRenderID, vec3(0.f, 0.f, 1.f));

	
}

// At least for now, coins are not being created as physics objects
void GameManager::createCoin(unsigned int coinIndex)
{
	Coin* newCoin = state.getCoin(coinIndex);
	MeshObject* coinMesh = meshInfo.getMeshPointer(COIN);

	unsigned int coin = renderer.generateObjectID();
	renderer.assignMeshObject(coin, coinMesh);
	renderer.assignMaterial(coin, &tsMat);
	renderer.assignColor(coin, vec3(1.f, 1.f, 0.f));

	newCoin->setRenderID(coin);
}

void GameManager::createMine()
{
	unsigned int powerup = renderer.generateObjectID();
	MeshObject* powerupMesh = meshInfo.getMeshPointer(MINE);

	renderer.assignMeshObject(powerup, powerupMesh);
	renderer.assignMaterial(powerup, &tsMat);
	renderer.assignTexture(powerup, powerupMesh->getTextureID());
	Mine newMine = Mine(3.0);
	
	vec3 playerPos = state.getPlayer(0)->getPos();
	vec3 playerForward = state.getPlayer(0)->getForward();

	// there is definitely a simpler way of doing this
	vec3 placementVec = playerPos - (playerForward);
	placementVec = placementVec - (playerForward);
	placementVec = placementVec - (playerForward);
	newMine.setPos(vec3((placementVec.x), (placementVec.y - 1), (placementVec.z)));
	newMine.setRenderID(powerup);
	state.addMine(newMine);
}

void GameManager::createBomb(unsigned int playerID)
{
	vec3 location = 5.0f * state.getPlayer(playerID)->getForward() + state.getPlayer(playerID)->getPos();
	Bomb b = Bomb(physics.createBomb(location, playerID), renderer.generateObjectID(), location);
	state.addPowerup(b);

	renderer.assignMeshObject(b.getRenderID(), meshInfo.getMeshPointer(BOMB));
	renderer.assignMaterial(b.getRenderID(), &tsMat);
	renderer.assignTexture(b.getRenderID(), meshInfo.getMeshPointer(BOMB)->getTextureID());
	state.pushEvent(new BombCreationEvent(location));
}

void GameManager::createPowerupBox(unsigned int objectID)
{
	unsigned int box = renderer.generateObjectID();
	MeshObject* boxMesh = meshInfo.getMeshPointer(CUBE);

	renderer.assignMeshObject(box, boxMesh);
	renderer.assignMaterial(box, &tsMat);
	renderer.assignTexture(box, boxMesh->getTextureID());
	
	state.getPowerupBox(objectID)->setRenderID(box);
	physics.createPowerupBox(boxMesh->getVertices(), state.getPowerupBox(objectID)->getPos());
}

void GameManager::createBoostPad(unsigned int objectID)
{
	MeshObject* boostMesh = meshInfo.getMeshPointer(BOOST);
	unsigned int boost = renderer.generateObjectID();

	renderer.assignMeshObject(boost, boostMesh);
	renderer.assignMaterial(boost, &tsMat);
	renderer.assignColor(boost, vec3(0.f, 1.f, 0.f));
	//renderer.assignTexture

	state.getBoostPad(objectID)->setRenderID(boost);
}

void GameManager::initMenus() {
	sound = SoundManager(state);
	sound.startMenuSong(state);
	
	int redIndex = 0;
	int greenIndex = 1;
	int blueIndex = 2;
	int purpleIndex = 3;

	int currentMenu = 0;
	int lastMenu = 2;

	unsigned int startScreen = LoadTexture("menus/KoB_StartScreen.bmp");
	unsigned int sScreen = _interface.generateComponentID();
	
	unsigned int levelSelectScreen = 0;
	unsigned int lScreen = 0;

	unsigned int carSelectScreen = 0;
	unsigned int cScreen = 0;

	unsigned int menuBackground = LoadTexture("menus/Background.bmp");
	unsigned int menu = _interface.generateComponentID();

	unsigned int p1Icon = 0;
	unsigned int Icon1 = 0;
	const float lSel1X = -0.4f;
	const float lSel1Y = -0.89f;
	const float lSel2X = 0.4f;
	const float lSel2Y = -0.89f;
	const float cSel1X = -0.6f;
	const float cSel2X = 0.2f;
	const float cSel1Y = -0.21f;
	const float cSel2Y = -0.92f;
	float p1Scale = 0.25f;
	float p1X = lSel1X;
	float p1Y = lSel1Y;
	const float iconWidth = 0.1f;

	// 0 = red, 1 = green, 2 = blue, 3 = purple
	int selectedColour = 0;

	unsigned int p2Icon = 0;
	unsigned int Icon2 = 0;

	unsigned int p3Icon = 0;
	unsigned int Icon3 = 0;

	unsigned int p4Icon = 0;
	unsigned int Icon4 = 0;
	

	while (currentMenu <= lastMenu) {
		renderer.clearDrawBuffers(vec3(1.f, 1.f, 1.f));


		Input in = input.getInput(1);
		in = smoothers[0].smooth(in, false);
		if ((in.menu || in.powerup || in.jump || in.horn || in.drift) && currentMenu == 0) {
			currentMenu++;
			_interface.clear();

			p1Icon = LoadTexture("menus/P1Icon.png");
			Icon1 = _interface.generateComponentID();

			levelSelectScreen = LoadTexture("menus/KoB_LevelScreen.bmp");
			lScreen = _interface.generateComponentID();

			menuBackground = LoadTexture("menus/Background.bmp");
			menu = _interface.generateComponentID();
		}
		else if (((in.jump && !in.isKeyboard) || (in.powerup && in.isKeyboard)) && currentMenu == 1) {
			currentMenu++;
			_interface.clear();

			p1X = -0.6f;
			p1Y = -0.21f;
			p1Scale = 0.15f;
			p1Icon = LoadTexture("menus/P1Icon.png");
			Icon1 = _interface.generateComponentID();

			carSelectScreen = LoadTexture("menus/KoB_CarScreen.bmp");
			cScreen = _interface.generateComponentID();

			menuBackground = LoadTexture("menus/Background.bmp");
			menu = _interface.generateComponentID();
		}
		else if (((in.jump && !in.isKeyboard) || (in.powerup && in.isKeyboard)) && currentMenu == lastMenu) {
			_interface.clear();

			gameInit();

			return;
		}

		if (currentMenu == 0) {
			_interface.assignSquare(sScreen);
			_interface.assignTexture(sScreen, startScreen, ComponentInfo::UP_TEXTURE);
			_interface.setDimensions(sScreen, 0.f, 0.f, 2.f, 2.f, ANCHOR::CENTER);
		}
		// if level select menu
		else if (currentMenu == 1) {
			if (p1X == lSel1X && in.turnR < -0.3f) {
				p1X = lSel2X;
			}
			else if (p1X == lSel2X && in.turnL < -0.3f) {
				p1X = lSel1X;
			}

			_interface.assignSquare(Icon1);
			_interface.assignTexture(Icon1, p1Icon, ComponentInfo::UP_TEXTURE);
			_interface.setDimensions(Icon1, p1X, p1Y, p1Scale, p1Scale, ANCHOR::CENTER);

			_interface.assignSquare(lScreen);
			_interface.assignTexture(lScreen, levelSelectScreen, ComponentInfo::UP_TEXTURE);
			_interface.setDimensions(lScreen, 0.f, 0.f, 2.f, 2.f, ANCHOR::CENTER);
		}
		// If car select menu
		else if (currentMenu == 2) {
			if (p1Y == cSel1Y && in.tiltBackward > 0.3f)
			{
				p1Y = cSel2Y;

				// Blue selected
				if (p1X == cSel1X) {
					selectedColour = 2;
				}
				// Purple selected
				else {
					selectedColour = 3;
				}
			}
			else if (p1Y == cSel2Y && in.tiltForward > 0.3f) {
				p1Y = cSel1Y;

				// if red is selected
				if (p1X == cSel1X) {
					selectedColour = 0;
				}
				// if green is selected
				else {
					selectedColour = 1;
				}
			}
			else if (p1X == cSel1X && in.turnR < -0.3f) {
				p1X = cSel2X;

				// if green is selected
				if (p1Y == cSel1Y) {
					selectedColour = 1;
				}
				// if purple is selected
				else {
					selectedColour = 3;
				}
			}
			else if (p1X == cSel2X && in.turnL < -0.3f) {
				p1X = cSel1X;

				// if red is selected
				if (p1Y == cSel1Y) {
					selectedColour = 0;
				}
				// if blue is selected
				else {
					selectedColour = 2;
				}
			}

			if (selectedColour == 0 && redIndex != 0) {
				meshInfo.buggyTexIDs[redIndex] = meshInfo.buggyTexIDs[0];
				playerColours[0] = 0;

				if (blueIndex == 0) {
					blueIndex = redIndex;
				}
				else if (greenIndex == 0) {
					greenIndex = redIndex;
				}
				else if (purpleIndex == 0) {
					purpleIndex = redIndex;
				}
				redIndex = 0;
			}
			else if (selectedColour == 1 && greenIndex != 0) {
				playerColours[greenIndex] = playerColours[0];
				playerColours[0] = 1;

				if (redIndex == 0) {
					redIndex = greenIndex;
				}
				else if (blueIndex == 0) {
					blueIndex = greenIndex;
				}
				else if (purpleIndex == 0) {
					purpleIndex = greenIndex;
				}
				greenIndex = 0;
			}
			else if (selectedColour == 2 && blueIndex != 0) {
				playerColours[blueIndex] = playerColours[0];
				playerColours[0] = 2;

				if (redIndex == 0) {
					redIndex = blueIndex;
				}
				else if (greenIndex == 0) {
					greenIndex = blueIndex;
				}
				else if (purpleIndex == 0) {
					purpleIndex = blueIndex;
				}
				blueIndex = 0;
			}
			else if (selectedColour == 3 && purpleIndex != 0) {
				playerColours[purpleIndex] = playerColours[0];
				playerColours[0] = 3;

				if (redIndex == 0) {
					redIndex = purpleIndex;
				}
				else if (greenIndex == 0) {
					greenIndex = purpleIndex;
				}
				else if (blueIndex == 0) {
					blueIndex = purpleIndex;
				}
				purpleIndex = 0;
			}

			_interface.assignSquare(Icon1);
			_interface.assignTexture(Icon1, p1Icon, ComponentInfo::UP_TEXTURE);
			_interface.setDimensions(Icon1, p1X, p1Y, p1Scale, p1Scale, ANCHOR::CENTER);

			_interface.assignSquare(cScreen);
			_interface.assignTexture(cScreen, carSelectScreen, ComponentInfo::UP_TEXTURE);
			_interface.setDimensions(cScreen, 0.f, 0.f, 2.f, 2.f, ANCHOR::CENTER);
		}

		_interface.assignSquare(menu);
		_interface.assignTexture(menu, menuBackground, ComponentInfo::UP_TEXTURE);
		_interface.setDimensions(menu, 1.f, -1.f, 4.f, 2.f, ANCHOR::BOTTOM_RIGHT);

		
		_interface.drawAll(&renderer);

		sound.updateMenuSong(state);

		//Swap buffers  
		glfwSwapBuffers(window);
		//Get and organize events, like keyboard and mouse input, window resizing, etc...  
		glfwPollEvents();
	}
}

void GameManager::gameLoop()
{
	vector<vec3> polygons;
	vector<vec3> edges;
	
	mat4 lineTransform;
	lineTransform[3][1] = -6.f;

	NavMesh nav;
	ai.nav.loadNavMesh("donutLevelNavMesh.obj");
	ai.nav.calculateImplicitEdges();
	ai.nav.navMeshToLines(&polygons, &edges);

	vector<vec3> path;
	

	clock.start();
	float timeProgressed = 0.f;
	unsigned int frameCount = 0;

	float frameTime = 1.f / 60.f;

	physics.startSim(frameTime);

	// 5 because of 4 players and the inital golden buggy which is not
	// a player
	Input inputs[MAX_PLAYERS];

	//Test code (Gives AI initial random path
	ai.testTarget = ai.getRandomTarget();
	
	if (!ai.findNewPath(1, state.getGoldenBuggy()->getPos(), false)) {
		//cout << endl;
	}

	vector<vec3> frontier;
	vector<vec3> paths;

	Camera freeCam;

	bool paused = false;

	unsigned int debugPathIterations = 0;

	timeb loopStart = clock.getCurrentTime();

	hasPowerup.push_back(false);

	unsigned int numScreens = input.getNumPlayers();
	
	renderer.splitScreenViewports(numScreens);


	while (!glfwWindowShouldClose(window))
	{

		//Get inputs from players/ai
		for (unsigned int i = 0; i < state.numberOfPlayers(); i++)
		{
			if ((state.getPlayer(i)->isAI()) && !paused)
			{
				frameCount++;
				if (frameCount > 30)
				{
					ai.findNewPath(i, state.getGoldenBuggy()->getPos(), !state.getPlayer(i)->isGoldenBuggy());
					frameCount = 0;
				}

				inputs[i] = ai.getInput(i, state);
			}
			else {
				inputs[i] = input.getInput(i + 1);
			}

			inputs[i] = smoothers[i].smooth(inputs[i], state.getPlayer(i)->getInAir());

			if(!paused)
				physics.handleInput(&inputs[i], state.getPlayer(i)->getPhysicsID());
		}



		//I'm leaving a comment here so that once we add powerups we change the pause key
		if (inputs[0].menu)
		{
			paused = !paused;

			if (paused)
			{
				freeCam = cam[0];
				freeCam.setCameraMode(FREEROAM_CAMERA);
				renderer.loadCamera(&freeCam);
				debugPathIterations = 0;
			}
			else {
				renderer.loadCamera(&cam[0]);


			}
		}

		//Not AI code. AIManager shouldn't change the golden buggy
		if (physics.newGoldenBuggy){
			physics.newGoldenBuggy = false;

			//Switch the golden buggie
			PlayerInfo* p = state.getPlayer(physics.indexOfGoldenBuggy);
			unsigned int chassisRenderId_goldenBuggy = p->getRenderID();
			renderer.assignTexture(chassisRenderId_goldenBuggy, meshInfo.getGoldenBuggyTexID());


			//Switch the player that used to be the golden buggy
			PlayerInfo* p_2 = state.getPlayer(physics.indexOfOldGoldenBuggy);
			int chasisRenderId_reg = p_2->getRenderID();
			// Switch buggy back to original colour
			unsigned int origTexture = p_2->getTextureID();
			renderer.assignTexture(chasisRenderId_reg, origTexture);

			state.setGoldenBuggy(physics.indexOfGoldenBuggy);
		}

		/*
		for (unsigned int i = 0; i < state.numberOfPlayers(); i++) {
			if (inputs[i].powerup) {
				createBomb(i);	
			}
		}
		*/

		//Allow for nitro/powerup activation her
		if (inputs[0].cheat_coin || inputs[0].powerup){

			//VehicleTraits traits = VehicleTraits(physics.getMaterial());
			//traits.print();


			//VehicleTraits temp = VehicleTraits(physics.getMaterial());
			//traits.loadConfiguration("base");
			//temp.print();

			//createDecoyGoldenBuggie(vec3(-5.f, 5.f, -15.f), traits);

			if (state.numberOfMines() < 20){
				//printf("cheated in placing a Mine \n");
				//createPowerup(MINE);
			}

			if (hasPowerup.at(0))
			{
				
				
				/*
				1 - Nitro Boost
				2 - Bomb
				3 - Mine
				4 - Coin?
				5 - Decoy?
				*/

				int powerUpId = state.getPlayer(0)->usePowerUp();
				
				if (powerUpId == POWERUPS::NITROBOOST)
				{
					physics.applyNitroBoost(0);//Revise this to allow for nitro to be a energy system
					printf("Nitro Boost \n");
				}
				else if (powerUpId == POWERUPS::BOMB)
				{
					createBomb(0);
					printf("Bomb \n");
				}
				else if (powerUpId == POWERUPS::MINE)
				{
					createMine();
					printf("Mine \n");
				}
				else if (powerUpId == POWERUPS::COIN)
				{
					physics.modifySpeed(0, 0.3333f);	//Should change this..
					printf("Coin? \n");
				}
				else if (powerUpId == POWERUPS::DECOY)
				{
					//Apply decoy powerup logic
					printf("Decoy? \n");
				}

				

				hasPowerup.at(0) = false;

			} 
			else {
			
				//Test code should eventually be removed
				/*
				int powerUpId = randomPowerup();
				if (powerUpId == POWERUPS::NITROBOOST)
				{
					state.getPlayer(0)->addPowerUp(powerUpId);
					printf("Nitro Boost \n");
				}
				else if (powerUpId == POWERUPS::BOMB)
				{
					state.getPlayer(0)->addPowerUp(powerUpId);
					printf("Bomb \n");
				}
				else if (powerUpId == POWERUPS::MINE)
				{
					state.getPlayer(0)->addPowerUp(powerUpId);
					createPowerup(MINE);
					printf("Mine \n");
				}
				else if (powerUpId == POWERUPS::COIN)
				{
					state.getPlayer(0)->addPowerUp(powerUpId);
					printf("Coin? \n");
				}
				else if (powerUpId == POWERUPS::DECOY)
				{
					state.getPlayer(0)->addPowerUp(powerUpId);
					printf("Decoy? \n");
				}
				*/
				//hasPowerup.at(0) = true;
			}
			
		}

		//Update game state and renderer
		physics.updateGameState(&state, frameTime);
		renderer.updateObjectTransforms(&state);
		sound.updateSounds(state, inputs);

		for (int i = 0; i < state.getNbEvents(); i++) {
			Event* e = state.getEvent(i);

			if (e->getType() == VEHICLE_BOMB_COLLISION_EVENT) {
				// Remove bomb
				VehicleBombCollisionEvent* powerupEvent = (VehicleBombCollisionEvent*)(e);
				Powerup* explodedBomb = state.getPowerup(powerupEvent->ob2);
				renderer.deleteDrawableObject(explodedBomb->getRenderID());
				state.removePowerup(powerupEvent->ob2);
			}
			else if (e->getType() == POWERUPBOX_COLLISION_EVENT)
			{
				PowerupCollisionEvent* powerupEvent = dynamic_cast<PowerupCollisionEvent*>(e);
				int vehicleId = powerupEvent->ob1;
				int powerupId = powerupEvent->ob2; //Delete the powerup, note the id is based on the order that the powerups are made.

				hasPowerup[vehicleId] = true;
				int powerUpType = randomPowerup();
				if (!state.getPlayer(vehicleId)->isGoldenBuggy() && powerUpType == POWERUPS::DECOY)
					powerUpType = POWERUPS::NITROBOOST;	//Prevents the non golden buggies from using the Decoy	
				state.getPlayer(vehicleId)->addPowerUp(powerUpType);

				printf("Player %d has powerup %d \n", vehicleId, powerUpType);
			}
		}
		state.clearEvents();

		//Test code...
		PlayerInfo* player = state.getPlayer(0);
		vec3 posPlayer = player->getPos();

		PlayerInfo* ai_state = state.getPlayer(1);
		vec3 posAI = ai_state->getPos();

		vec3 diff = posAI - posPlayer;

		if (!paused) {
			// Check for player/coin collisions, and coin respawns also check for other collisions
			for (unsigned int i = 0; i < state.numberOfPlayers(); i++) {
				bool hasCoinCollision = state.checkCoinCollision(state.getPlayer(i)->getPos());
				bool hasBoostPadCollision = state.checkBoostPadCollision(state.getPlayer(i)->getPos());
				int hasMineCollision = state.checkMineCollision(state.getPlayer(i)->getPos());
				if (hasCoinCollision){
					//TODO change to all
					physics.modifySpeed(i, 0.3333f);
					sound.playDingSound(state.getPlayer(i)->getPos());
				}

				if (hasBoostPadCollision){
					physics.applySpeedPadBoost(i);
				}

				if (hasMineCollision > -1){
					printf("Mine Explosion! \n");
					physics.applyMineExplosion(i);
					renderer.deleteDrawableObject(state.getMine(hasMineCollision)->getRenderID());
					state.removeMine(hasMineCollision);
				}
			}
			state.checkCoinRespawns();
			state.applyRotations();
		}

		

		float scale = 0.1f;

		if (!paused)
		{
			//Physics sim 
			physics.getSim();

			physics.startSim(frameTime);
		}
		//Free camera movement
		if (paused)
		{
			//Debugging avoidance
			ai.debugAIPath(&paths, 1, debugPathIterations);
			if (inputs[0].jump)
				debugPathIterations++;

			freeCam.rotateView(-inputs[0].camH*scale, -inputs[0].camV*scale);
			freeCam.move(vec3(inputs[0].turnL - inputs[0].turnR, 0, inputs[0].forward - inputs[0].backward));
		}

		
		if (inputs[0].menu) {
			displayDebugging = !displayDebugging;
		}

		//Draw scene
		renderer.clearDrawBuffers(vec3(1.f, 1.f, 1.f));

		for (unsigned int i = 0; i < numScreens; i++)
		{
			//Free camera movement
			if (paused && (i == 0))
			{
				renderer.loadCamera(&freeCam);
				//Debugging avoidance
				ai.debugAIPath(&paths, 1, debugPathIterations);
				if (inputs[0].jump)
					debugPathIterations++;

				freeCam.rotateView(-inputs[0].camH*scale, -inputs[0].camV*scale);
				freeCam.move(vec3(inputs[0].turnL - inputs[0].turnR, 0, inputs[0].forward - inputs[0].backward));
			}
			else
			{
				renderer.loadCamera(&cam[i]);

				//Update camera position
				PlayerInfo* activePlayer = state.getPlayer(i);
				vec4 cameraPosition = physics.vehicle_getGlobalPose(
					activePlayer->getPhysicsID())*vec4(0.f, 0.f, 0.f, 1.f);
				vec3 cPos = vec3(cameraPosition) / cameraPosition.w;

				if (cPos != cam[i].getViewCenter())
					cam[i].changeCenterAndPos(cPos - cam[i].getViewCenter());

				//Update to accomodate more players and multiple cameras
				cam[i].rotateView(inputs[i].camH*scale, inputs[i].camV*scale);

				cameraEnvironmentCollision(&cam[i]);

				//Track camera around front of vehicle
				vec4 carDir = physics.vehicle_getGlobalPose(activePlayer->getPhysicsID())*vec4(0.f, 0.f, 1.f, 0.f);
				if ((inputs[i].camH == 0.f) && (inputs[i].camV == 0.f))
					cam[i].trackDirAroundY(vec3(carDir), frameTime);
			}

			

			//Translate skydome
			mat4 translation;
			translation[3][0] = state.getPlayer(i)->getPos().x;
			translation[3][1] = state.getPlayer(i)->getPos().y;
			translation[3][2] = state.getPlayer(i)->getPos().z;
			renderer.assignTransform(skyboxID, translation);

			//Render
			renderer.useViewport(i+1);
			renderer.drawAll();
			renderer.drawUI(_interface.generateScoreBars(&state), vehicleColours);
			renderer.drawRadar(state.setupRadar(0));

			//Debugging
			if (displayDebugging)
			{
				ai.getPathAsLines(1, &path);

				renderer.drawLines(polygons, vec3(0.f, 1.f, 0.f), lineTransform);
				renderer.drawLines(path, vec3(1.f, 0.f, 0.f), lineTransform);
				//renderer.drawLines(edges, vec3(0.f, 0.f, 1.f), lineTransform);
			}

			if (paused)
			{
				renderer.drawLines(paths, vec3(0.7f, 0.5f, 1.f), lineTransform);
			}

			_interface.drawAll(&renderer);
		}
		

		
		
		if (!paused) {
			// increase score and check win conditions
			state.getGoldenBuggy()->incrementScore();
			unsigned int theScore = state.getGoldenBuggy()->getScore();
			if ((theScore % 100) == 0) {
				std::printf("Player %i score: %i\n", state.getGoldenBuggyID(), state.getGoldenBuggy()->getScore());
			}
			if (theScore >= state.getMaxScore() && gameOver == false) {
				winner = state.getGoldenBuggyID();
				printf("Player %i is the winner!\n", winner);
				if (winner == 0) {
					sound.playWinSound(state.getPlayer(0)->getPos());
				}
				else {
					sound.playLossSound(state.getPlayer(0)->getPos());
				}
				gameOver = true;
				//break;
			}
		}

		//Get path
		path.clear();

		//Swap buffers  
		glfwSwapBuffers(window);
		//Get and organize events, like keyboard and mouse input, window resizing, etc...  
		glfwPollEvents();

		//Wait until end of frame
		float timeElapsed = clock.getElapsedTime();
		if (frameTime > timeElapsed)
			clock.waitUntil(frameTime - timeElapsed);
		
		//Display FPS
		timeProgressed += max(timeElapsed, frameTime);
		frameCount++;
		if (timeProgressed > 1.f)
		{
			printf("FPS = %d\n", frameCount);
			frameCount = 0;
			timeProgressed = 0.f;
		}

		clock.start();
	}
	quitGame(winner);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void GameManager::gameInit()
{
	gameOver = false;
	
	// temporary since we only have one level right now
	createLevel(DONUTLEVEL);
	MeshObject* levelMesh = meshInfo.getMeshPointer(DONUTLEVEL);
	state.setMap(levelMesh, "models\\levelinfo\\donutlevelcoinlocations.obj", "models\\levelinfo\\donutlevelboostlocations.obj", "models\\levelinfo\\donutlevelboxlocations.obj");

	initTestScene();
}

void GameManager::initTestScene()
{

	for (int i = 0; i < playerColours.size(); i++) {
		if (playerColours[i] == 0) {
			vehicleColours.push_back(vec3(1.f, 0.f, 0.f)); // red car

		}
		else if (playerColours[i] == 1) {
			vehicleColours.push_back(vec3(0.f, 1.f, 0.f)); // green car
		}
		else if (playerColours[i] == 2) {
			vehicleColours.push_back(vec3(0.f, 0.f, 1.f)); // blue
		}
		else {
			// Black car sucks. it's purple now
			vehicleColours.push_back(vec3(1.f, 0.f, 1.f)); // purple
		}
	}
	
	// Yellow car also sucks, so now it's orange
	vehicleColours.push_back(vec3(1.f, 0.64f, 0.f)); // orange


	VehicleTraits traits = VehicleTraits(physics.getMaterial());
	traits.print();

	unsigned int TOO_MANY = 0;

	VehicleTraits temp = VehicleTraits(physics.getMaterial());
	traits.loadConfiguration("base");
	temp.print();

	if (playerColours[0] == 0) {
		createPlayer(vec3(0.f, 5.f, 0.f), traits, meshInfo.getRedBuggyTexID());
	}
	else if (playerColours[0] == 1) {
		createPlayer(vec3(0.f, 5.f, 0.f), traits, meshInfo.getGreenBuggyTexID());
	}
	else if (playerColours[0] == 2) {
		createPlayer(vec3(0.f, 5.f, 0.f), traits, meshInfo.getBlueBuggyTexID());
	}
	else if (playerColours[0] == 3) {
		createPlayer(vec3(0.f, 5.f, 0.f), traits, meshInfo.getPurpleBuggyTexID());
	}


	if (playerColours[1] == 0) {
		createPlayer(vec3(-5.f, 5.f, -15.f), traits, meshInfo.getRedBuggyTexID());
	}
	else if (playerColours[1] == 1) {
		createPlayer(vec3(-5.f, 5.f, -15.f), traits, meshInfo.getGreenBuggyTexID());
	}
	else if (playerColours[1] == 2) {
		createPlayer(vec3(-5.f, 5.f, -15.f), traits, meshInfo.getBlueBuggyTexID());
	}
	else if (playerColours[1] == 3) {
		createPlayer(vec3(-5.f, 5.f, -15.f), traits, meshInfo.getPurpleBuggyTexID());
	}

	if (playerColours[2] == 0) {
		createPlayer(vec3(-5.f, 5.f, 15.f), traits, meshInfo.getRedBuggyTexID());
	}
	else if (playerColours[2] == 1) {
		createPlayer(vec3(-5.f, 5.f, 15.f), traits, meshInfo.getGreenBuggyTexID());
	}
	else if (playerColours[2] == 2) {
		createPlayer(vec3(-5.f, 5.f, 15.f), traits, meshInfo.getBlueBuggyTexID());
	}
	else if (playerColours[2] == 3) {
		createPlayer(vec3(-5.f, 5.f, 15.f), traits, meshInfo.getPurpleBuggyTexID());
	}

	if (playerColours[3] == 0) {
		createPlayer(vec3(5.f, 5.f, -15.f), traits, meshInfo.getRedBuggyTexID());
	}
	else if (playerColours[3] == 1) {
		createPlayer(vec3(5.f, 5.f, -15.f), traits, meshInfo.getGreenBuggyTexID());
	}
	else if (playerColours[3] == 2) {
		createPlayer(vec3(5.f, 5.f, -15.f), traits, meshInfo.getBlueBuggyTexID());
	}
	else if (playerColours[3] == 3) {
		createPlayer(vec3(5.f, 5.f, -15.f), traits, meshInfo.getPurpleBuggyTexID());
	}

	for (unsigned int i = 0; i < TOO_MANY; i++)
	{
		createPlayer(vec3(5.f, 5.f, -15.f), traits, meshInfo.getBuggyTexID(0));
	}

	
	for (unsigned int i = input.getNumPlayers(); i < MAX_PLAYERS - 1; i++)
	{
		state.getPlayer(i)->setAI(true);
	}

	for (unsigned int i = 4; i < TOO_MANY + 4; i++)
	{
		state.getPlayer(i)->setAI(true);
	}
	
	//Create skybox
	skyboxTextureID = LoadTexture("textures/sky_photo6.jpg");
	skyboxID = renderer.generateObjectID();
	renderer.assignSkyDome(skyboxID, 70.f, 50, &skyboxVerts, &skyboxUVs, &skyboxIndices, skyboxTextureID);
	renderer.assignMaterial(skyboxID, &skyMaterial);

	state.setGoldenBuggy(0);
	sound.stopMenuSong();
	sound.initListener(state);
	sound.startSounds(state);



	// Add all coins as render objects
	for (unsigned int i = 0; i < state.numberOfCoins(); i++) { createCoin(i); }

	for (unsigned int i = 0; i < state.numberOfBoostPads(); i++) { createBoostPad(i); }

	for (unsigned int i = 0; i < state.numberOfPowerupBoxes(); i++) { createPowerupBox(i); }

	vec3 lightPos(60.f, 60.f, 60.f);
	unsigned int lightID = renderer.generateLightObject();
	renderer.setLightPosition(lightID, lightPos);

	ai.initAI();







	//Add dummy objects to interface
	
	/*
	carSelectScreen = LoadTexture("menus/KoB_CarScreen.bmp");
	unsigned int centerBox = _interface.generateComponentID();
	_interface.assignSquare(centerBox);


	_interface.assignTexture(centerBox, carSelectScreen, ComponentInfo::UP_TEXTURE);
	_interface.setDimensions(centerBox, 1.f, -1.f, 1.f, 1.f, ANCHOR::BOTTOM_RIGHT);
	*/

	
	//createPlayer(vec3(0.f, 5.f, 3.f)); //SHOULD BE AI methods

	
}

int GameManager::randomPowerup()
{
	/*
	1 - Nitro Boost
	2 - Bomb 
	3 - Mine
	4 - Coin?
	5 - Decoy?

	*/
	/* initialize random seed: */
	

	int powerupId = rand() % 4;
	return powerupId;
}

void GameManager::quitGame(unsigned int winnerID)
{

}

#endif // GAMEMANAGER_CPP