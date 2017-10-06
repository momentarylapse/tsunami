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

class CurveConsole : public SideBarConsole
{
public:
	CurveConsole(AudioView *view, Song *song);
	virtual ~CurveConsole();

	void onViewChange();
	void onUpdate();

	void updateList();
	void onAdd();
	void onDelete();
	void onTarget();
	void onListEdit();
	void onListSelect();
	void onEditSong();
	void onEditTrack();
	void onEditFx();

	virtual void onEnter();
	virtual void onLeave();

	Song *song;
	AudioView *view;
	Curve* curve();

	string id_list;
};

#endif /* CURVECONSOLE_H_ */
