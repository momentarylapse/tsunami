/*
 * ActionSongSetDefaultFormat.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_DATA_ACTIONSONGSETDEFAULTFORMAT_H_
#define SRC_ACTION_SONG_DATA_ACTIONSONGSETDEFAULTFORMAT_H_

#include "../../Action.h"
#include "../../../Data/Audio/AudioBuffer.h"


class ActionSongSetDefaultFormat : public Action {
public:
	ActionSongSetDefaultFormat(SampleFormat format, int compression);

	void *execute(Data *d) override;
	void undo(Data *d) override;

	SampleFormat format;
	int compression;
};

#endif /* SRC_ACTION_SONG_DATA_ACTIONSONGSETDEFAULTFORMAT_H_ */
