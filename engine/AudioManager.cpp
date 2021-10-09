#include "AudioManager.h"

#include "AudioWorldEmitter.h"

#include "stdc++.h"

#include "Debugger.h"

#define FADE_TIME 1.0f

#define SFXMODIF   AudioManager::masterGlobalVolume * AudioManager::SFXGlobalVolume
#define ENVMODIF   AudioManager::masterGlobalVolume * AudioManager::environmentGlobalVolume
#define MUSICMODIF AudioManager::masterGlobalVolume * AudioManager::musicGlobalVolume

float AudioManager::masterGlobalVolume = 1.0f;
float AudioManager::musicGlobalVolume = 1.0f;
float AudioManager::SFXGlobalVolume = 1.0f;
float AudioManager::environmentGlobalVolume = 1.0f;

float randomClamp(float min, float max) {
	if (min == max) return min;
	float val = (float)Rand() / (float)RAND_MAX;
	return min + val * (max - min);
}
float changeSemitone(float frequency, float variation) {
	static float semitone_ratio = pow(2.0f, 1.0f / 12.0f);
	return frequency * pow(semitone_ratio, variation);
}

void MusicManager::play(const std::string& path) {
	if (currentSongPath == path) return;
	if (currentSong != 0) {
		stopSongs();
		nextSongPath = path;
		return;
	}	auto sound = soundMap->find(path); 
	if (sound == soundMap->end()) return; 
	if (system->playSound(sound->second, group, true, &currentSong) == FMOD_OK) {
		currentSongPath = path;
		currentSong->setVolume(MUSICMODIF);
		currentSong->setPaused(false);
		state = FADE_IN;
	}
}

void MusicManager::stopSongs() {
	if (currentSong) state = FADE_OUT;
	nextSongPath.clear();
}

void MusicManager::manage(float delta) {
	if (currentSong) {
		if (state == FADE_IN) {
			float volume; currentSong->getVolume(&volume);
			volume = (std::min)(1.0f, volume + delta / FADE_IN);
			if (volume == 1.0f) state = NONE;
			currentSong->setVolume(volume * MUSICMODIF);
		}	else
		if (state == FADE_OUT) {
			float volume; currentSong->getVolume(&volume);
			volume = (std::max)(0.0f, volume - delta / FADE_IN);
			if (volume == 0.0f) state = NONE, currentSong->stop(), currentSong = 0, currentSongPath.clear();
			else currentSong->setVolume(volume * MUSICMODIF);
		}
	}	else
	if (!nextSongPath.empty())
		play(nextSongPath), nextSongPath.clear();
}


AudioManager::AudioManager() : receptive_x(0), receptive_y(0) {
	FMOD::System_Create(&system);
	system->init(100, FMOD_INIT_NORMAL, 0);
	if (system) {
		system->getMasterChannelGroup(&groups[CHMASTER]);
		for (int i = 1; i < CHCOUNT; ++i)
			system->createChannelGroup(0, &groups[i]), MASTER->addGroup(groups[i]);
	}

	modes[CHSFX] = FMOD_DEFAULT;
	modes[CHMUSIC] = FMOD_DEFAULT | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL;
	modes[CHEMITTER] = FMOD_DEFAULT;

	musicManager = new MusicManager();
	if (musicManager) {
		musicManager->soundMap = &soundMap[CHMUSIC];
		musicManager->system = system;
		musicManager->group = groups[CHMUSIC];
	}
	
	for (auto& name : getNamesInsideFolder("sounds/sfx"))              load(CHSFX,     name);
	for (auto& name : getNamesInsideFolder("sounds/music"))            load(CHMUSIC,   name);
	for (auto& name : getNamesInsideFolder("sounds/environment"))	   load(CHEMITTER, name);
}

AudioManager::~AudioManager() {
	for (int i = 1; i < CHCOUNT; soundMap[i].clear(), ++i)
		for (auto& it:soundMap[i]) it.second->release();
	if (system) system->release(), system = 0;
}

void AudioManager::addWorldEmitter(AudioWorldEmitter* emitter) {
	if (emitter) {
		FMOD::Channel *channel;
		if (system->playSound(soundMap[CHEMITTER][emitter->path], groups[CHEMITTER], true, &channel) == FMOD_OK) {
			emitterMap[emitter] = channel;
			channel->setMode(FMOD_LOOP_NORMAL);
			emitter->adjustSoundChannel(channel, receptive_x, receptive_y, ENVMODIF);
			channel->setPaused(false);
		}
	}
}
void AudioManager::refreshEmitters() {
	auto it = emitterMap.begin();
	bool bit;
	while (it != emitterMap.end())
		if (it->first == nullptr || it->first->wasDestroyed() || it->second->isPlaying(&bit) == FMOD_ERR_INVALID_HANDLE)
			it->second->stop(), emitterMap.erase(it++);
		else it->first->adjustSoundChannel(it->second, receptive_x, receptive_y, ENVMODIF), ++ it;
}

void AudioManager::manage(float delta) {
	refreshEmitters();
	if (system) system->update();
	if (musicManager) musicManager->manage(delta);
}

void AudioManager::playSFX(const std::string& path, SFXBehaviour behaviour) {
	auto it = soundMap[CHSFX].find(path);
	if (it == soundMap[CHSFX].end()) return;
	FMOD::Channel *channel;
	if (system)
		system->playSound(it->second, groups[CHSFX], true, &channel), 
		behaviour.apply(channel);
}
void AudioManager::playMusic(const std::string& path) {
	if (musicManager) musicManager->play(path);
}

void AudioManager::load(CHANNEL_CATEGORY chtype, const std::string& path) {
	if (soundMap[chtype].find(path) != soundMap[chtype].end()) return;
	FMOD::Sound *sound;
	std::string identif = path.substr(path.rfind('\\') + 1, path.size());
	if (system && system->createSound(path.c_str(), modes[chtype], 0, &sound) == FMOD_OK)
		soundMap[chtype][identif] = sound;
}
void AudioManager::stop(CHANNEL_CATEGORY chtype) {
	if (chtype != CHMUSIC) {
		if (groups[chtype]) groups[chtype]->stop();
		return;
	}	if (musicManager) musicManager->stopSongs();
}
void AudioManager::setVolume(CHANNEL_CATEGORY chtype, float value) {
	if (groups[chtype]) groups[chtype]->setVolume(value);
}


void SFXBehaviour::apply(FMOD::Channel* channel) {
	channel->setVolume(randomClamp(volume[0], volume[1]) * SFXMODIF);
	float freq; channel->getFrequency(&freq);
	channel->setFrequency(changeSemitone(freq, randomClamp(pitch[0], pitch[1])));
	channel->setPaused(false);
}
