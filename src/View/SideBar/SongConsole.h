/*
 * SongConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SONGCONSOLE_H_
#define SONGCONSOLE_H_

#include "SideBar.h"
class Song;

class SongConsole: public SideBarConsole
{
public:
	SongConsole(Song *s);
	virtual ~SongConsole();

	void loadData();
	void applyData();

	void onSamplerate();
	void onFormat();
	void onCompression();
	void onTrackList();
	void onTagsSelect();
	void onTagsEdit();
	void onAddTag();
	void onDeleteTag();

	void onEditLayers();
	void onEditSamples();
	void onEditFx();

	void onUpdate(Observable *o);

	Song *song;
};

#endif /* SONGCONSOLE_H_ */
