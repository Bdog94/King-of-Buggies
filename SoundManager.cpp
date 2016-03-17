#ifndef SOUNDMANAGER_CPP
#define SOUNDMANAGER_CPP
#include "SoundManager.h"

SoundManager::SoundManager() {
}



SoundManager::SoundManager(GameState state) {
	initOpenAL(state);
	for (int i = 0; i < state.numberOfPlayers(); i++) {
		honking.push_back(false);
		honkSources.push_back(0);
	}
}

void SoundManager::initOpenAL(GameState state) {
	ALCdevice* device = alcOpenDevice(NULL);
	if (!device) {
		cout << "ALCDevice creation failed\n";
	}
	ALCcontext* context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);

	initListener(state);
	startMusic(state);
	startEngineSounds(state);
}

/*
 * Initializes the listener at the location of the player
 */
void SoundManager::initListener(GameState state) {
	PlayerInfo* p1 = state.getPlayer(0);

	ALfloat *ListenerPos = vec3ToALfloat(p1->getPos()).data();
	ALfloat *ListenerVel = vec3ToALfloat(p1->getVelocity()).data();
	ALfloat *ListenerOri = vec3ToALfloat(p1->getForward(), p1->getUp()).data();

	alListenerfv(AL_POSITION, ListenerPos);
	alListenerfv(AL_VELOCITY, ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);
}

/*
 * Initializes the music to be playing at the location of the player
 */
void SoundManager::startMusic(GameState state) {
	ALuint buffer;

	loadWavToBuf("Dogsong.wav", &musicSource, &buffer);

	PlayerInfo* p1 = state.getPlayer(0);

	ALfloat *SourcePos = vec3ToALfloat(p1->getPos()).data();
	ALfloat *SourceVel = vec3ToALfloat(p1->getVelocity()).data();

	alSourcei(musicSource, AL_BUFFER, buffer);
	alSourcef(musicSource, AL_PITCH, 1.0f);
	alSourcef(musicSource, AL_GAIN, musicVolume);
	alSourcefv(musicSource, AL_POSITION, SourcePos);
	alSourcefv(musicSource, AL_VELOCITY, SourceVel);
	alSourcei(musicSource, AL_LOOPING, AL_TRUE);

	alSourcePlay(musicSource);
}

/*
 * Initializes engines sounds to play at the location of players
 */
void SoundManager::startEngineSounds(GameState state) {
	for (unsigned int i = 0; i < state.numberOfPlayers(); i++) {
		ALuint buffer;

		loadWavToBuf("Idle.wav", &engineSources[i], &buffer);

		PlayerInfo* player = state.getPlayer(i);

		ALfloat *SourcePos = vec3ToALfloat(player->getPos()).data();
		ALfloat *SourceVel = vec3ToALfloat(player->getVelocity()).data();

		alSourcei(engineSources[i], AL_BUFFER, buffer);
		alSourcef(engineSources[i], AL_PITCH, 1.0f);
		alSourcef(engineSources[i], AL_GAIN, idleEngineVolume);
		alSourcefv(engineSources[i], AL_POSITION, SourcePos);
		alSourcefv(engineSources[i], AL_VELOCITY, SourceVel);
		alSourcei(engineSources[i], AL_LOOPING, AL_TRUE);

		alSourcePlay(engineSources[i]);
	}
}


/*
 * Updates the location of the listener so it is still the location of the player
 */
void SoundManager::updateListener(GameState state) {
	PlayerInfo* p1 = state.getPlayer(0);

	ALfloat *ListenerPos = vec3ToALfloat(p1->getPos()).data();
	ALfloat *ListenerVel = vec3ToALfloat(p1->getVelocity()).data();
	ALfloat *ListenerOri = vec3ToALfloat(p1->getForward(), p1->getUp()).data();

	alListenerfv(AL_POSITION, ListenerPos);
	alListenerfv(AL_VELOCITY, ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);
}

/*
 * Updates the music so it is still playing at the location of the player
 */
void SoundManager::updateMusic(GameState state) {
	PlayerInfo* p1 = state.getPlayer(0);

	ALfloat *SourcePos = vec3ToALfloat(p1->getPos()).data();
	ALfloat *SourceVel = vec3ToALfloat(p1->getVelocity()).data();

	alSourcefv(musicSource, AL_POSITION, SourcePos);
	alSourcefv(musicSource, AL_VELOCITY, SourceVel);
}

/*
 * Updates the engines sounds' location as well as volume and pitch based
 * on input
 */
void SoundManager::updateEngineSounds(GameState state, Input inputs[]) {
	for (unsigned int i = 0; i < state.numberOfPlayers(); i++) {
		PlayerInfo* player = state.getPlayer(i);
	
		bool forwardsGear = player->getForwardsGear();

		float accelInput = 0;
		accelInput = abs(player->getEngineSpeed());

		ALfloat *SourcePos = vec3ToALfloat(player->getPos()).data();
		ALfloat *SourceVel = vec3ToALfloat(player->getVelocity()).data();

		float pitchMod = map(accelInput, 0, 600, 1, 2.0);
		if (pitchMod < 1) {
			pitchMod = 1;
		}
		alSourcef(engineSources[i], AL_PITCH, idleEnginePitch * pitchMod);
		

		accelInput = map(accelInput, 0, 600, 1, 16);
		alSourcef(engineSources[i], AL_GAIN, idleEngineVolume * accelInput);
		alSourcefv(engineSources[i], AL_POSITION, SourcePos);
		alSourcefv(engineSources[i], AL_VELOCITY, SourceVel);
		alSourcei(engineSources[i], AL_LOOPING, AL_TRUE);
	}
}

void SoundManager::updateHonks(GameState state, Input inputs[]) {
	for (int i = 0; i < state.numberOfPlayers(); i++) {
		if (inputs[i].horn) {
			if (!honking[i]) {

				ALuint buffer;

				loadWavToBuf("Honk.wav", &honkSources[i], &buffer);

				PlayerInfo* player = state.getPlayer(i);

				ALfloat *SourcePos = vec3ToALfloat(player->getPos()).data();
				ALfloat *SourceVel = vec3ToALfloat(player->getVelocity()).data();

				alSourcei(honkSources[i], AL_BUFFER, buffer);
				alSourcef(honkSources[i], AL_PITCH, 1.0);
				alSourcef(honkSources[i], AL_GAIN, 1.0);
				alSourcefv(honkSources[i], AL_POSITION, SourcePos);
				alSourcefv(honkSources[i], AL_VELOCITY, SourceVel);
				alSourcei(honkSources[i], AL_LOOPING, AL_TRUE);

				alSourcePlay(honkSources[i]);
				honking[i] = true;
			}
			else {
				PlayerInfo* p1 = state.getPlayer(i);

				ALfloat *SourcePos = vec3ToALfloat(p1->getPos()).data();
				ALfloat *SourceVel = vec3ToALfloat(p1->getVelocity()).data();

				alSourcefv(honkSources[i], AL_POSITION, SourcePos);
				alSourcefv(honkSources[i], AL_VELOCITY, SourceVel);
			}
		}
		else {
			if (honking[i]) {
				alSourcePause(honkSources[i]);
				honking[i] = false;
			}
		}
	}
}

/*
 * Converts a vec3 to an array of ALfloats
 */
vector<ALfloat> SoundManager::vec3ToALfloat(vec3 vec) {
	vector<ALfloat> f = { vec.x, vec.y, vec.z };
	return f;
}

/*
 * Converts two vec3s to an array of ALfloats
 */
 vector<ALfloat> SoundManager::vec3ToALfloat(vec3 vector1, vec3 vector2) {
	vector<ALfloat> f = { vector1.x, vector1.y, vector1.z, vector2.x, vector2.y, vector2.z };
	return f;
}

/*
 * Loads a wav file to the given buffer and loads that buffer into the source
 * No need to add "Sounds\" to the file name. This method will do that
 * Just give "sound.wav"
 */
void SoundManager::loadWavToBuf(string fileName, ALuint* source, ALuint* buffer) {
	string name = "Sounds/" + fileName;
	const char* fileNameChar = name.c_str();
	FILE* fp = fopen(fileNameChar, "rb");
	char type[4];
	DWORD size, chunkSize;
	short formatType, channels;
	DWORD sampleRate, avgBytesPerSec;
	short bytesPerSample, bitsPerSample;
	DWORD dataSize;

	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F')
	{
		cout << "No RIFF\n";
	}

	fread(&size, sizeof(DWORD), 1, fp);
	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'W' || type[1] != 'A' || type[2] != 'V' || type[3] != 'E')
	{
		cout << "Not WAVE\n";
	}

	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'f' || type[1] != 'm' || type[2] != 't' || type[3] != ' ')
	{
		cout << "Not fmt\n";
	}

	fread(&chunkSize, sizeof(DWORD), 1, fp);
	fread(&formatType, sizeof(short), 1, fp);
	fread(&channels, sizeof(short), 1, fp);
	fread(&sampleRate, sizeof(DWORD), 1, fp);
	fread(&avgBytesPerSec, sizeof(DWORD), 1, fp);
	fread(&bytesPerSample, sizeof(short), 1, fp);
	fread(&bitsPerSample, sizeof(short), 1, fp);

	fread(type, sizeof(char), 4, fp);
	if (type[0] != 'd' || type[1] != 'a' || type[2] != 't' || type[3] != 'a')
	{
		cout << "Missing DATA\n";
	}

	fread(&dataSize, sizeof(DWORD), 1, fp);

	unsigned char *buf = new unsigned char[dataSize];
	fread(buf, sizeof(BYTE), dataSize, fp);

	ALuint frequency = sampleRate;
	ALenum format = 0;

	alGenBuffers(1, buffer);
	alGenSources(1, source);

	if (bitsPerSample == 8) {
		if (channels == 1) {
			format = AL_FORMAT_MONO8;
		}
		else if (channels == 2) {
			format = AL_FORMAT_STEREO8;
		}
	}
	else if (bitsPerSample == 16) {
		if (channels == 1) {
			format = AL_FORMAT_MONO16;
		}
		else if (channels == 2) {
			format = AL_FORMAT_STEREO16;
		}
	}

	alBufferData(*buffer, format, buf, dataSize, frequency);
}

/*
 * Loads and plays a bump sound once at the given position
 */
void SoundManager::playBumpSound(vec3 pos, float volume) {
	playSound("Bump.wav", pos, volume);
}

/*
* Loads and plays a bump sound once at the given position
*/
void SoundManager::playDingSound(vec3 pos) {
	ALuint dingSource;
	ALuint buffer;

	loadWavToBuf("Ding.wav", &dingSource, &buffer);

	ALfloat *SourcePos = vec3ToALfloat(pos).data();

	alSourcei(dingSource, AL_BUFFER, buffer);
	alSourcef(dingSource, AL_PITCH, 1.0f);
	alSourcef(dingSource, AL_GAIN, 1.0f);
	alSourcefv(dingSource, AL_POSITION, SourcePos);
	alSourcei(dingSource, AL_LOOPING, AL_FALSE);

	alSourcePlay(dingSource);
}

void SoundManager::playSecretMusic(GameState state) {
	alSourcePause(musicSource);
	ALuint buffer;
	int songSelection = rand() % 5;

	if (songSelection == 0) {
		loadWavToBuf("Secret.wav", &musicSource, &buffer);
	}
	else if (songSelection == 1) {
		loadWavToBuf("Dogstorm.wav", &musicSource, &buffer);
	}
	else if (songSelection == 2) {
		loadWavToBuf("Dogbass.wav", &musicSource, &buffer);
	}
	else if (songSelection == 3) {
		loadWavToBuf("DogLevels.wav", &musicSource, &buffer);
	}
	else {
		loadWavToBuf("DogsongMetal.wav", &musicSource, &buffer);
	}
	

	PlayerInfo* p1 = state.getPlayer(0);

	ALfloat *SourcePos = vec3ToALfloat(p1->getPos()).data();
	ALfloat *SourceVel = vec3ToALfloat(p1->getVelocity()).data();

	alSourcei(musicSource, AL_BUFFER, buffer);
	alSourcef(musicSource, AL_PITCH, 1.0f);
	alSourcef(musicSource, AL_GAIN, musicVolume);
	alSourcefv(musicSource, AL_POSITION, SourcePos);
	alSourcefv(musicSource, AL_VELOCITY, SourceVel);
	alSourcei(musicSource, AL_LOOPING, AL_FALSE);

	alSourcePlay(musicSource);
}

void SoundManager::updateMusicPitch(GameState state, Input input) {
	PlayerInfo* player = state.getPlayer(0);

	bool forwardsGear = player->getForwardsGear();

	float accelInput = 0;
	if (forwardsGear) {
		accelInput = map(input.forward, 0, 1, 0.5, 2);
	}
	else {
		accelInput = map(input.backward, 0, 1, 0.5, 2);
	}

	alSourcef(musicSource, AL_PITCH, 1 * accelInput);
}

void SoundManager::playWinSound(vec3 pos) {
	playSound("Yay.wav", pos, 1.0f);
}

void SoundManager::playLossSound(vec3 pos) {
	playSound("Trombone.wav", pos, 1.0f);
}

void SoundManager::playSound(string fileName, vec3 pos, float volume) {
	ALuint source;
	ALuint buffer;

	loadWavToBuf(fileName, &source, &buffer);

	ALfloat *SourcePos = vec3ToALfloat(pos).data();

	alSourcei(source, AL_BUFFER, buffer);
	alSourcef(source, AL_PITCH, 1.0f);
	alSourcef(source, AL_GAIN, volume);
	alSourcefv(source, AL_POSITION, SourcePos);
	alSourcei(source, AL_LOOPING, AL_FALSE);

	alSourcePlay(source);
}

/*
 * Updates all sounds
 */
void SoundManager::updateSounds(GameState state, Input inputs[]) {
	updateListener(state);
	updateMusic(state);
	updateEngineSounds(state, inputs);

	updateHonks(state, inputs);

	if (inputs[0].konamiCode) {
		playSecretMusic(state);
	}

	ALint musicState;
	alGetSourcei(musicSource, AL_SOURCE_STATE, &musicState);
	if (musicState == AL_STOPPED) {
		alSourcePause(musicSource);
		startMusic(state);
	}

	for (int i = 0; i < state.getNbEvents(); i++) {
		Event* e = state.getEvent(i);

		if (e->getType() == VEHICLE_COLLISION_EVENT)
		{
			VehicleCollisionEvent* vehE = dynamic_cast<VehicleCollisionEvent*>(e);

			vec3 force = vehE->force;
			float volume = length(force);
			volume = map(volume, 0, 300000, 0, 2);
			cout << "Volume: " << volume << "\n";
			playBumpSound(vehE->location, volume);
		}
		else if (e->getType() == VEHICLE_WALL_COLLISION_EVENT) {
			VehicleWallCollisionEvent* vehE = dynamic_cast<VehicleWallCollisionEvent*>(e);

			vec3 force = vehE->force;
			float volume = length(force);
			volume = map(volume, 0, 300000, 0, 2);
			cout << "Volume: " << volume << "\n";
			playBumpSound(vehE->location, volume);
		}
		else if (e->getType() == GOLDEN_BUGGY_SWITCH_EVENT) {
			GoldenBuggySwitchEvent* gbE = dynamic_cast<GoldenBuggySwitchEvent*>(e);
			playSound("Boom.wav", gbE->gbPos, 1.0f);
		}
	}
	// Feel free to remove this. It has no purpose, just sounds funny
	//updateMusicPitch(state, inputs[0]);
}

/**
* Takes a value between two numbers and converts it to another value between two different numbers
* maintaining its proportional place between the first two numbers.
*/
float SoundManager::map(float value, float min, float max, float newMin, float newMax) {
	return (value - min) * ((newMax - newMin) / (max - min)) + newMin;
}

#endif
