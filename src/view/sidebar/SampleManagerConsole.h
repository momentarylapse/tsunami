/*
 * SampleManagerConsole.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef SAMPLEMANAGERCONSOLE_H_
#define SAMPLEMANAGERCONSOLE_H_

#include "SideBar.h"

namespace tsunami {

class Song;
class Sample;
class SampleManagerItem;
class Session;
class SamplePreviewPlayer;

class SampleManagerConsole : public SideBarConsole {
public:
	SampleManagerConsole(Session *session, SideBar *bar);

	obs::sink in_song_update;

	void on_enter() override;
	void on_leave() override;

	void update_list();

	void on_list_edit();
	void on_list_right_click();
	void on_import();
	void on_export();
	void on_preview();
	void on_insert();
	void on_create_from_selection();
	void on_auto_delete();
	void on_delete();
	void on_scale();

	void on_song_update();

	owned_array<SampleManagerItem> items;
	Array<SampleManagerItem*> old_items;
	void add(SampleManagerItem *item);
	void remove(SampleManagerItem *item);
	int get_index(Sample *s);
	Array<Sample*> get_selected();

	void set_selection(const Array<Sample*> &samples);
	
	string id_list;
	owned<hui::Menu> menu_samples;
	bool editing_cell = false;
};

}

#endif /* SAMPLEMANAGERCONSOLE_H_ */
