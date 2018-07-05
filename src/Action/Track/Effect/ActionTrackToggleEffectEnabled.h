/*
 * ActionTrackToggleEffectEnabled.h
 *
 *  Created on: 30.03.2014
 *      Author: michi
 */

#ifndef ACTIONTRACKTOGGLEEFFECTENABLED_H_
#define ACTIONTRACKTOGGLEEFFECTENABLED_H_

#include "../../Action.h"
class AudioEffect;

class ActionTrackToggleEffectEnabled: public Action
{
public:
	ActionTrackToggleEffectEnabled(AudioEffect *fx);

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	AudioEffect *fx;
};

#endif /* ACTIONTRACKTOGGLEEFFECTENABLED_H_ */
