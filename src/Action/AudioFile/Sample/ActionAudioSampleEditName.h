/*
 * ActionAudioSampleEditName.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONAUDIOSAMPLEEDITNAME_H_
#define ACTIONAUDIOSAMPLEEDITNAME_H_

#include "../../ActionMergable.h"

class AudioFile;

class ActionAudioSampleEditName: public ActionMergable<string>
{
public:
	ActionAudioSampleEditName(AudioFile *a, int index, const string &name);
	virtual ~ActionAudioSampleEditName();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	int index;
};

#endif /* ACTIONAUDIOSAMPLEEDITNAME_H_ */
