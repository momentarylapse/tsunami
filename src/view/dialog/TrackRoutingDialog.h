/*
 * TrackRoutingDialog.h
 *
 *  Created on: 11.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_TRACKROUTINGDIALOG_H_
#define SRC_VIEW_DIALOG_TRACKROUTINGDIALOG_H_

#include "../../lib/hui/hui.h"

namespace tsunami {

class Song;
class Track;

class TrackRoutingDialog : public hui::Dialog {
public:
	TrackRoutingDialog(hui::Window *parent, Song *song);
	void load();

	void on_target(int i);
	void on_add_group();

	Array<Track*> groups;
	Song *song;
	int num_tracks;
};

}

#endif /* SRC_VIEW_DIALOG_TRACKROUTINGDIALOG_H_ */
