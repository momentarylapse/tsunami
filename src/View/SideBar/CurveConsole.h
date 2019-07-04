/*
 * CurveConsole.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef CURVECONSOLE_H_
#define CURVECONSOLE_H_

#include "SideBar.h"
#include "../../lib/math/math.h"

class Song;
class AudioView;
class Curve;
class Session;

class CurveConsole : public SideBarConsole {
public:
	CurveConsole(Session *session);
	virtual ~CurveConsole();

	void on_update();

	void update_list();
	void on_add();
	void on_delete();
	void on_target();
	void on_list_edit();
	void on_list_select();
	void on_edit_song();
	void on_edit_track();
	void on_edit_fx();

	void on_enter() override;
	void on_leave() override;

	Curve* curve();
	Array<Curve*> curves;

	string id_list;
};

#endif /* CURVECONSOLE_H_ */
