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

class BaseFxConsole : public SideBarConsole
{
public:
	BaseFxConsole(const string &title, Session *session);
	virtual ~BaseFxConsole();

	void on_enter() override;
	void on_leave() override;
	void on_set_large(bool large) override;

	void set_exclusive(hui::Panel *p);
	hui::Panel *exclusive;
	bool allow_show(hui::Panel *p);

	string id_inner;

	Array<hui::Panel*> panels;
};

class FxConsole : public BaseFxConsole
{
public:
	FxConsole(Session *session);
	virtual ~FxConsole();

	void clear();
	void set_track(Track *t);

	void on_add();

	void on_edit_song();
	void on_edit_track();

	void on_track_delete();
	void on_view_cur_track_change();
	void on_update();

	Track *track;
};

class GlobalFxConsole : public BaseFxConsole
{
public:
	GlobalFxConsole(Session *session);
	virtual ~GlobalFxConsole();

	void clear();
	void update();

	void on_add();

	void on_edit_song();

	void on_update();
};

#endif /* FXCONSOLE_H_ */
