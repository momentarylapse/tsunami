/*
 * ActionAudioSetDefaultFormat.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOSETDEFAULTFORMAT_H_
#define SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOSETDEFAULTFORMAT_H_

#include "../../Action.h"
#include "../../../Data/BufferBox.h"


class ActionAudioSetDefaultFormat : public Action
{
public:
	ActionAudioSetDefaultFormat(SampleFormat format, int compression);
	virtual ~ActionAudioSetDefaultFormat(){}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	SampleFormat format;
	int compression;
};

#endif /* SRC_ACTION_AUDIOFILE_DATA_ACTIONAUDIOSETDEFAULTFORMAT_H_ */
