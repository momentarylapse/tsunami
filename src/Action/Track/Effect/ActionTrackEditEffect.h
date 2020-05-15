/*
 * ActionTrackEditEffect.h
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#ifndef ACTIONTRACKEDITEFFECT_H_
#define ACTIONTRACKEDITEFFECT_H_

#include "../../ActionMergable.h"
class AudioEffect;

class ActionTrackEditEffect: public ActionMergable<string> {
public:
	ActionTrackEditEffect(AudioEffect *fx);

	void *execute(Data *d) override;
	void undo(Data *d) override;
	void redo(Data *d) override;

	bool mergable(Action *a) override;

private:
	AudioEffect *fx;
};

#endif /* ACTIONTRACKEDITEFFECT_H_ */
