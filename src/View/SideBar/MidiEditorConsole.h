/*
 * MidiEditorConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIEDITORCONSOLE_H_
#define MIDIEDITORCONSOLE_H_

#include "SideBar.h"
#include "../../Stuff/Observer.h"
#include "../../lib/math/math.h"

class Song;
class AudioView;
class Track;

class MidiEditorConsole : public SideBarConsole, public Observer
{
public:
	MidiEditorConsole(AudioView *view, Song *audio);
	virtual ~MidiEditorConsole();

	virtual void onEnter();
	virtual void onLeave();

	virtual void onUpdate(Observable *o, const string &message);
	void update();

	void onScale();
	void onBeatPartition();
	void onViewModeMidi();
	void onViewModeClassical();
	void onViewModeTab();
	void onCreationMode();
	void onInterval();
	void onChordType();
	void onChordInversion();
	void onReferenceTracks();
	void onModifierNone();
	void onModifierSharp();
	void onModifierFlat();
	void onModifierNatural();

	void onEditTrack();
	void onEditMidiFx();
	void onEditSong();

	void clear();
	void setTrack(Track *t);


	string id_inner;

	AudioView *view;
	Track *track;
	Song *song;
};

#endif /* MIDIEDITORCONSOLE_H_ */
