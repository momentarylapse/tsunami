/*
 * FxList.h
 *
 *  Created on: 31.03.2012
 *      Author: michi
 */

#ifndef FXLIST_H_
#define FXLIST_H_

#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"

class FxList : public HuiEventHandler
{
public:
	FxList(CHuiWindow *_dlg, const string &_id, Array<Effect> &_fx);
	virtual ~FxList();

	void FillList();
	void OnList();

	bool UpdateEffectParams(Effect &f, bool deletable);
	void AddNewEffect(string &filename);
	void ExecuteFXDialog(int index);

public:
	CHuiWindow *dlg;
	string id;
	Array<Effect> &fx;
};

#endif /* FXLIST_H_ */
