#include "AudioWorldEmitter.h"

#include "GodClass.h"
#include "AudioManager.h"

#include "stdc++.h"

#define MAXPAN_ABS 1.0f
#define MINVOL     0.2f

AudioWorldEmitter::AudioWorldEmitter(const std::string& path, bool bLoop, float duration) : path(path), duration(duration), bLoop(bLoop) {
	x = y = 0;
	source = this;
	AUDIO->addWorldEmitter(this);
}

void AudioWorldEmitter::loop(float delta) {
	PrimitiveObject::loop(delta);
	if (source == nullptr || source->wasDestroyed())
		destroy();
	if (duration != 0.0f && lifespan > duration)
		destroy();
}

void AudioWorldEmitter::adjustSoundChannel(FMOD::Channel* ch, int receptive_x, int receptive_y, float scale) {
	float volume = 1.0f, panning = 0.0f;
	if (receptive_x < left(focus))
		computeSoundChannelData(ch, receptive_x, receptive_y, volume, panning, -1);
	else if (receptive_x > right(focus))
		computeSoundChannelData(ch, receptive_x, receptive_y, volume, panning, 1);

	ch->setVolume(volume * scale);
	ch->setPan(panning);
}

void AudioWorldEmitter::computeSoundChannelData(FMOD::Channel* ch, int receptive_x, int receptive_y, float& volume, float& pan, int sgn) {
	bool b1 = (receptive_x <= right(focus) && receptive_x >= left(focus));
	bool b2 = (receptive_y >= up   (focus) && receptive_y <= down(focus));
	if (b1 & b2) { volume = 1.0f, pan = .0f; return; }
	if (receptive_x < left(accomodation) || receptive_x > right(accomodation) || receptive_y < up(accomodation) || receptive_y > down(accomodation)) { volume = 0.0f, pan = .0f; return; }

	float diff_x = 1.0f, diff_y = 1.0f;
	float ratio_x = 1.0f, ratio_y = 1.0f;

	diff_x = (float)(left(focus) - left(accomodation));
	diff_y = (float)(up  (focus) - up  (accomodation));
	if (diff_x == .0f && !b1) { volume = 0.0f, pan = .0f; return; }
	if (diff_y == .0f && !b2) { volume = 0.0f, pan = .0f; return; }

	if (!b1) {
		if (receptive_x < left(focus)) ratio_x = (std::max)(0, receptive_x - left(accomodation))/diff_x;
		else						   ratio_x = (std::max)(0, right(accomodation) - receptive_x)/diff_x;
	}
	if (!b2) {
		if (receptive_y < up(focus))   ratio_y = (std::max)(0, receptive_y - up(accomodation)) / diff_y;
		else                           ratio_y = (std::max)(0, down(accomodation) - receptive_y) / diff_y;
	}

	volume = MINVOL + (1.0f - MINVOL) * std::min(ratio_x, ratio_y);
	pan = sgn * (MAXPAN_ABS * (1 - ratio_x));
}


