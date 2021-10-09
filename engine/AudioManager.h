#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>
#include <list>
#include <map>

#include <fmod.hpp>

struct SFXBehaviour {
	SFXBehaviour(float minVolume = 1.0f, float maxVolume = 1.0f, float minPitch = 1.0f, float maxPitch = 1.0f) {
		volume[0] = minVolume, volume[1] = maxVolume;
		pitch[0] = minPitch, pitch[1] = maxPitch;
	}	float volume[2], pitch[2];
	void apply(FMOD::Channel* channel);
}; 

class MusicManager {
public:
	enum FADE_STATE { NONE = 0, FADE_IN, FADE_OUT };
	FADE_STATE state;

private:
	MusicManager() { currentSong = nullptr; currentSongPath = nextSongPath = ""; }

	void manage(float delta);

	void play(const std::string& path);
	void stopSongs();

	FMOD::System       *system;
	FMOD::Channel      *currentSong;
	FMOD::ChannelGroup *group;

	std::string currentSongPath;
	std::string nextSongPath;
	std::map <std::string, FMOD::Sound*>* soundMap;

	friend class AudioManager;
};

enum CHANNEL_CATEGORY { CHMASTER = 0, CHSFX, CHMUSIC, CHEMITTER, CHCOUNT };

#define MASTER groups[CHMASTER]

class AudioWorldEmitter;

class AudioManager
{
public:
	AudioManager();
	virtual ~AudioManager();

	void playSFX  (const std::string &path, SFXBehaviour behaviour = SFXBehaviour());
	void playMusic(const std::string &path);

	void load	  (CHANNEL_CATEGORY chtype, const std::string &path);
	void stop     (CHANNEL_CATEGORY chtype);
	void setVolume(CHANNEL_CATEGORY chtype, float value);

	void addWorldEmitter(AudioWorldEmitter *emitter);

protected:
	virtual void manage(float delta);

	static float masterGlobalVolume;
	static float musicGlobalVolume;
	static float environmentGlobalVolume;
	static float SFXGlobalVolume;

private:
	int receptive_x, receptive_y;

	FMOD::System	   *system;
	FMOD::ChannelGroup *groups[CHCOUNT];
	FMOD_MODE		    modes[CHCOUNT];

	std::map <std::string, FMOD::Sound*> soundMap[CHCOUNT];
	std::map <AudioWorldEmitter*, FMOD::Channel*> emitterMap;

	MusicManager *musicManager;

	void refreshEmitters();

	friend class MusicManager;
	friend struct SFXBehaviour;
	friend class GodClass;
	friend class Debugger;
};

#endif