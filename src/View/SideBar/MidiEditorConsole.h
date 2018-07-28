/*
 * MidiEditorConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIEDITORCONSOLE_H_
#define MIDIEDITORCONSOLE_H_

#include "SideBar.h"

class TrackLayer;
enum class MidiMode;

class MidiEditorConsole : public SideBarConsole
{
public:
	MidiEditorConsole(Session *session);
	virtual ~MidiEditorConsole();

	virtual void onEnter();
	virtual void onLeave();

	void onLayerDelete();
	void onViewCurLayerChange();
	void onViewVTrackChange();
	void on_settings_change();
	void update();

	void onScale();
	void onBeatPartition();
	void onNoteLength();
	void onCreationMode();
	void onInterval();
	void onChordType();
	void onChordInversion();
	void onReferenceTracks();
	void onModifierNone();
	void onModifierSharp();
	void onModifierFlat();
	void onModifierNatural();

	void onQuantize();

	void onEditTrack();
	void onEditMidiFx();
	void onEditSong();

	void clear();
	void setLayer(TrackLayer *t);


	string id_inner;

	TrackLayer *layer;
};

#endif /* MIDIEDITORCONSOLE_H_ */
