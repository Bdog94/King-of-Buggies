#ifndef GAMEMANAGER_CPP
#define GAMEMANAGER_CPP

#include "GameManager.h"

unsigned char testTexture[12] = {	0, 0, 255,
									0, 255, 0,
									255, 0, 0,
									255, 255, 255};

GameManager::GameManager(GLFWwindow* newWindow) : renderer(newWindow), input(newWindow), state(), physics(), 
	cam(vec3(0.0, 0.0, -1.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 5.0), MODELVIEWER_CAMERA), _interface()
{
	window = newWindow;
	mat = Diffuse();
	shinyMat = Specular(20.f);
	tsMat = TorranceSparrow(10.f);
	matteMat = TorranceSparrow(0.5f);
	skyMaterial = Unshaded();

	timePassedDecoy = 0;

	renderer.loadPerspectiveTransform(0.1f, 100.f, 80.f);		//Near, far, fov
	renderer.loadCamera(&cam);

	ai = AIManager(&state);

	//TODO: Put this indexing somewhere useful;
	ai.initAI(1);

	// setup interface according to window dimensions
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	_interface.setWindowDim(width, height);

	gameInit();
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

void GameManager::createPlayer(vec3 position, VehicleTraits traits)
{
	//Make argument to function later

	unsigned int chassisRenderID = renderer.generateObjectID();
	unsigned int physicsID = physics.vehicle_create(traits, position);

	vec3 colour = vehicleColours[state.numberOfPlayers()];
	unsigned int texID = meshInfo.getBuggyTexID(state.numberOfPlayers());

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
	smoothers.push_back(InputSmoother());
	

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

void GameManager::createPowerup(unsigned int objectID)
{
	unsigned int powerup = renderer.generateObjectID();
	//state.getPowerup(i)->setRenderID(powerup);

	MeshObject powerupMesh = meshInfo.getMesh(objectID);
	vector<vec3> powerupVerts = powerupMesh.getVertices();
	vector<vec3> powerupNormals = powerupMesh.getNormals();
	vector<unsigned int> powerupIndices = powerupMesh.getIndices();

	renderer.assignMesh(powerup, &powerupVerts);
	renderer.assignNormals(powerup, &powerupNormals);
	renderer.assignIndices(powerup, &powerupIndices);
	renderer.assignMaterial(powerup, &tsMat);
	//renderer.assignTexture()
}

void GameManager::createBoostPad(vec3 position)
{
	BoostPad newBoostPad;
	newBoostPad.setDefault(position);
	//Set up the mesh here...

	//renderer stuff here

	state.addBoostPad(newBoostPad);

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
	Input inputs[5];

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

				inputs[i] = ai.getInput(i);
			}
			else {
				inputs[i] = input.getInput(i + 1);
			}

			inputs[i] = smoothers[i].smooth(inputs[i], state.getPlayer(i)->getInAir());

			if(!paused)
				physics.handleInput(&inputs[i], state.getPlayer(i)->getPhysicsID());
		}



		//I'm leaving a comment here so that once we add powerups we change the pause key
		if (inputs[0].powerup)
		{
			paused = !paused;

			if (paused)
			{
				freeCam = cam;
				freeCam.setCameraMode(FREEROAM_CAMERA);
				renderer.loadCamera(&freeCam);
				debugPathIterations = 0;
			}
			else
				renderer.loadCamera(&cam);
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

		

		

		//Update game state and renderer
		physics.updateGameState(&state, frameTime);
		renderer.updateObjectTransforms(&state);
		sound.updateSounds(state, inputs);
		state.clearEvents();

		//Test code...
		PlayerInfo* player = state.getPlayer(0);
		vec3 posPlayer = player->getPos();

		PlayerInfo* ai_state = state.getPlayer(1);
		vec3 posAI = ai_state->getPos();

		vec3 diff = posAI - posPlayer;


		// Check for player/coin collisions, and coin respawns also check for other collisions
		for (unsigned int i = 0; i < state.numberOfPlayers(); i++) {
			bool hasCoinCollision = state.checkCoinCollision(state.getPlayer(i)->getPos());
			bool hasBoostPadCollision = state.checkBoostPadCollision(state.getPlayer(i)->getPos());
			bool hasMineCollision = state.checkMineCollision(state.getPlayer(i)->getPos());
			if (hasCoinCollision){
				//TODO change to all
				physics.modifySpeed(i, 0.3333f);
				sound.playDingSound(state.getPlayer(i)->getPos());
			}

			if (hasBoostPadCollision){
				//physics.applySpeedPadBoost(i);
			}

			if (hasMineCollision){
				printf("Mine Explosion! \n");
				//physics.applyMineExplosion(i);
			}
		}


		//Allow for nitro/powerup activation her
		if (inputs[0].cheat_coin){
			
			inputs[0].cheat_coin = false;

			//
			VehicleTraits traits = VehicleTraits(physics.getMaterial());
			//traits.print();


			//VehicleTraits temp = VehicleTraits(physics.getMaterial());
			//traits.loadConfiguration("base");
			//temp.print();

			//createDecoyGoldenBuggie(vec3(-5.f, 5.f, -15.f), traits);
			
			if (state.numberOfMines() < 20){
				//input[0].cheat_coin = false;
				printf("cheated in placing a Mine \n");
				Mine newMine = Mine(3.0, (state.getPlayer(0)->getPos() - vec3(5, 0, 0)));
				state.addMine(newMine);

				//TEST
				//physics.applyMineExplosion(0);
			}
		
		
		}

		//Update camera position
		PlayerInfo* activePlayer = state.getPlayer(0);
		vec4 cameraPosition = physics.vehicle_getGlobalPose(
			activePlayer->getPhysicsID())*vec4(0.f, 0.f, 0.f, 1.f);
		vec3 cPos = vec3(cameraPosition) / cameraPosition.w;

		if (cPos != cam.getViewCenter())
			cam.changeCenterAndPos(cPos - cam.getViewCenter());

		float scale = 0.1f;

		if (!paused)
		{
			//Physics sim 
			physics.getSim();

			physics.startSim(frameTime);

			//Update to accomodate more players and multiple cameras
			cam.rotateView(inputs[0].camH*scale, inputs[0].camV*scale);

			cameraEnvironmentCollision(&cam);

			//Track camera around front of vehicle
			vec4 carDir = physics.vehicle_getGlobalPose(activePlayer->getPhysicsID())*vec4(0.f, 0.f, 1.f, 0.f);
			if ((inputs[0].camH == 0.f) && (inputs[0].camV == 0.f))
				cam.trackDirAroundY(vec3(carDir), frameTime);

			//Translate skydome
			mat4 translation;
			translation[3][0] = state.getPlayer(0)->getPos().x;
			translation[3][1] = state.getPlayer(0)->getPos().y;
			translation[3][2] = state.getPlayer(0)->getPos().z;
			renderer.assignTransform(skyboxID, translation);
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

		if (inputs[0].menu)
			displayDebugging = !displayDebugging;

		//Draw scene
		renderer.clearDrawBuffers();
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
		
		//printf("player score: %d\n", _interface.getScoreBarWidth(&state));

		// increase score and check win conditions
		state.getGoldenBuggy()->incrementScore();
		int theScore = state.getGoldenBuggy()->getScore();
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
	state.setMap(levelMesh, "models\\levelinfo\\donutlevelcoinlocations.obj");

	initTestScene();
}

void GameManager::initTestScene()
{
	vehicleColours.push_back(vec3(1.f, 0.f, 0.f)); // red car
	vehicleColours.push_back(vec3(0.f, 1.f, 0.f)); // green car
	vehicleColours.push_back(vec3(0.f, 0.f, 1.f)); // blue
	vehicleColours.push_back(vec3(1.f, 1.f, 0.f)); // yellow
	// Black car sucks. it's purple now
	vehicleColours.push_back(vec3(1.f, 0.f, 1.f)); // purple

	VehicleTraits traits = VehicleTraits(physics.getMaterial());
	traits.print();


	VehicleTraits temp = VehicleTraits(physics.getMaterial());
	traits.loadConfiguration("base");
	temp.print();

	createPlayer(vec3(0.f, 5.f, 0.f), traits);
	createPlayer(vec3(-5.f, 5.f, -15.f), traits);
	state.getPlayer(1)->setAI(true);
	
	//Create skybox
	skyboxTextureID = LoadTexture("textures/sky_photo6.jpg");
	skyboxID = renderer.generateObjectID();
	renderer.assignSkyDome(skyboxID, 70.f, 50, &skyboxVerts, &skyboxUVs, &skyboxIndices, skyboxTextureID);
	renderer.assignMaterial(skyboxID, &skyMaterial);

	state.setGoldenBuggy(0);

	// Add all coins as render objects
	for (unsigned int i = 0; i < state.numberOfCoins(); i++) {
		createCoin(i);
	}

	//createBoostPad(vec3(10.f, -0.3f, 10.f));
	//createBoostPad(vec3(10.f, -0.3f, 10.f));
	

	vec3 lightPos(60.f, 60.f, 60.f);
	unsigned int lightID = renderer.generateLightObject();
	renderer.setLightPosition(lightID, lightPos);

	ai.initAI();
	sound = SoundManager(state);
	
	//createPlayer(vec3(0.f, 5.f, 3.f)); //SHOULD BE AI methods

	
}

void GameManager::quitGame(unsigned int winnerID)
{

}



/*


void GameManager::createPowerupBoxes()
{
int i = 0;
while (i < NUM_POWERBOXES) {
unsigned int box = renderer.generateObjectID();
state.getPowerupBox(i)->setRenderID(box);

// leave commented for now, there's no powerup meshes
//MeshObject coinMesh = meshInfo.getMesh(POWERUP);
MeshObject boxMesh = meshInfo.getMesh(SPHERE);
vector<vec3> boxVerts = boxMesh.getVertices();
vector<vec3> boxNormals = boxMesh.getNormals();
vector<unsigned int> boxIndices = boxMesh.getIndices();

renderer.assignMesh(box, &boxVerts);
renderer.assignNormals(box, &boxNormals);
renderer.assignIndices(box, &boxIndices);
renderer.assignMaterial(box, &tsMat);
i++;
}
}
*/

#endif // GAMEMANAGER_CPP