/*
 * ActionSongSetDefaultFormat.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_SONG_DATA_ACTIONSONGSETDEFAULTFORMAT_H_
#define SRC_ACTION_SONG_DATA_ACTIONSONGSETDEFAULTFORMAT_H_

#include "../../../Data/AudioBuffer.h"
#include "../../Action.h"


class ActionSongSetDefaultFormat : public Action
{
public:
	ActionSongSetDefaultFormat(SampleFormat format, int compression);
	virtual ~ActionSongSetDefaultFormat(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	SampleFormat format;
	int compression;
};

#endif /* SRC_ACTION_SONG_DATA_ACTIONSONGSETDEFAULTFORMAT_H_ */
