/*
 * MarkerDialog.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_VIEW_DIALOG_MARKERDIALOG_H_
#define SRC_VIEW_DIALOG_MARKERDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../Data/Range.h"

class TrackLayer;
class TrackMarker;

class MarkerDialog: public hui::Window {
public:
	MarkerDialog(hui::Window *_parent, TrackLayer *t, const Range &range, const string &text, const TrackMarker *marker);
	MarkerDialog(hui::Window *_parent, TrackLayer *t, const Range &range, const string &text);
	MarkerDialog(hui::Window *_parent, TrackLayer *t, const TrackMarker *marker);

	void on_edit();
	void on_ok();
	void on_close();

	enum class Mode {
		TEXT,
		KEY
	} mode;

	TrackLayer *layer;
	Range range;
	string text;
	const TrackMarker *marker;
};

#endif /* SRC_VIEW_DIALOG_MARKERDIALOG_H_ */
