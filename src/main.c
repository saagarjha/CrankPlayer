#include "pd_api.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

PlaydateAPI *playdate;
LCDVideoPlayer *video_player;
FilePlayer *forward_player;
FilePlayer *backward_player;
FilePlayer *current_player;
float framerate;
float length;
int frames;
bool initialized;

static float player_offset() {
	float offset = playdate->sound->fileplayer->getOffset(current_player);
	return current_player == forward_player ? offset : length - offset;
}

// Getting this to feel right is surprisingly difficult
static float normalize_rate(float rate) {
	float base = fabsf(rate);
	if (base >= 15) {
		base = 1;
	} else if (base >= 1) {
		base = 0.5;
	} else {
		base = 0;
	}
	return ((rate > 0) - (rate < 0)) * base;
}

static int callback(void *userdata) {
	// this is a hack, since apparently you can't set an offset outside of here
	if (!initialized) {
		playdate->sound->fileplayer->setOffset(backward_player, length - playdate->sound->fileplayer->getOffset(forward_player));
		initialized = true;
	}

	float rate = normalize_rate(playdate->system->getCrankChange());
	FilePlayer *newPlayer = current_player;
	if (rate > 0) {
		newPlayer = forward_player;
	} else if (rate < 0) {
		newPlayer = backward_player;
	}

	if (newPlayer != current_player) {
		playdate->sound->fileplayer->pause(current_player);
		playdate->sound->fileplayer->setOffset(newPlayer, length - playdate->sound->fileplayer->getOffset(current_player));
		playdate->sound->fileplayer->play(newPlayer, 1);
	}
	playdate->sound->fileplayer->setRate(current_player = newPlayer, fabsf(rate));
	current_player = newPlayer;
	int frame = player_offset() * framerate;

	if (0 <= frame && frame <= frames) {
		playdate->graphics->video->renderFrame(video_player, frame);
	}
	return 1;
}

int eventHandler(PlaydateAPI *_playdate, PDSystemEvent event, uint32_t arg) {
	switch (event) {
	case kEventInit:
		playdate = _playdate;
		playdate->system->setUpdateCallback(callback, NULL);

		video_player = playdate->graphics->video->loadVideo("video");
		playdate->graphics->video->useScreenContext(video_player);
		playdate->graphics->video->getInfo(video_player, NULL, NULL, &framerate, &frames, NULL);
		playdate->display->setRefreshRate(framerate);

		forward_player = playdate->sound->fileplayer->newPlayer();
		playdate->sound->fileplayer->loadIntoPlayer(forward_player, "audio");
		playdate->sound->fileplayer->setRate(forward_player, 0);
		playdate->sound->fileplayer->play(forward_player, 1);
		length = playdate->sound->fileplayer->getLength(forward_player);
		current_player = forward_player;

		backward_player = playdate->sound->fileplayer->newPlayer();
		playdate->sound->fileplayer->loadIntoPlayer(backward_player, "audio_reversed");
		playdate->sound->fileplayer->setRate(backward_player, 0);
		playdate->sound->fileplayer->play(backward_player, 1);
		playdate->sound->fileplayer->pause(backward_player);
		break;
	case kEventTerminate:
		playdate->graphics->video->freePlayer(video_player);
		playdate->sound->fileplayer->freePlayer(forward_player);
		playdate->sound->fileplayer->freePlayer(backward_player);
		break;
	default:
		break;
	}
	return 0;
}
