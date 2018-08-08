/*
 * FxConsole.h
 *
 *  Created on: 20.03.2014
 *      Author: michi
 */

#ifndef FXCONSOLE_H_
#define FXCONSOLE_H_

#include "SideBar.h"

class Track;
class Song;
class AudioView;
class Session;
class AudioEffect;

class FxConsole : public SideBarConsole
{
public:
	FxConsole(Session *session);
	virtual ~FxConsole();

	void on_enter() override;
	void on_leave() override;
	void on_set_large(bool large) override;

	void clear();
	void set_track(Track *t);

	void on_add();

	void on_edit_song();
	void on_edit_track();

	void on_track_delete();
	void on_view_cur_track_change();
	void on_update();

	void set_exclusive(AudioEffect *fx);
	AudioEffect *exclusive;
	bool allow_show(AudioEffect *fx);

	string id_inner;

	Track *track;
	Array<hui::Panel*> panels;
};

#endif /* FXCONSOLE_H_ */
