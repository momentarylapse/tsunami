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

namespace tsunami {

class Song;
class Track;
class AudioView;
class Curve;
class CurveTarget;
enum class CurveType;
class Session;

class CurveConsole : public SideBarConsole {
public:
	CurveConsole(Session *session, SideBar *bar);

	obs::sink in_update;
	obs::sink in_cur_layer_changed;

	void on_update();

	void update_list();
	void on_delete();
	void on_type(CurveType type);
	void on_list_edit();
	void on_list_select();
	void on_list_right_click();

	void on_enter() override;
	void on_leave() override;

	Curve* curve();
	Array<CurveTarget> targets;

	string id_list;
	Track *track();

	owned<hui::Menu> popup_menu;
};

}

#endif /* CURVECONSOLE_H_ */
