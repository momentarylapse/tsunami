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
	FxList(CHuiWindow *_dlg, const string &_id, const string &_id_add, const string &_id_edit, const string &_id_delete, Array<Effect> *_fx);
	virtual ~FxList();

	void FillList();
	void OnList();
	void OnListSelect();
	void OnAdd();
	void OnEdit();
	void OnDelete();

	bool UpdateEffectParams(Effect &f);
	void AddNewEffect(string &filename);
	void ExecuteFXDialog(int index);

	void SetFxList(Array<Effect> *fx);

public:
	CHuiWindow *dlg;
	string id;
	string id_add, id_edit, id_delete;
	Array<Effect> *fx;
};

#endif /* FXLIST_H_ */
