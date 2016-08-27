/*
 * ActionSongSampleEditName.h
 *
 *  Created on: 28.03.2014
 *      Author: michi
 */

#ifndef ACTIONSONGSAMPLEEDITNAME_H_
#define ACTIONSONGSAMPLEEDITNAME_H_

#include "../../ActionMergable.h"

class Song;
class Sample;

class ActionSongSampleEditName: public ActionMergable<string>
{
public:
	ActionSongSampleEditName(Song *a, Sample *s, const string &name);
	virtual ~ActionSongSampleEditName();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

	virtual bool mergable(Action *a);

private:
	Sample *sample;
};

#endif /* ACTIONSONGSAMPLEEDITNAME_H_ */
