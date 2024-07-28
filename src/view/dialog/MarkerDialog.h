/*
 * MarkerDialog.h
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#ifndef SRC_VIEW_DIALOG_MARKERDIALOG_H_
#define SRC_VIEW_DIALOG_MARKERDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../data/Range.h"

namespace tsunami {

class TrackLayer;
class TrackMarker;

class MarkerDialog: public hui::Dialog {
public:
	MarkerDialog(hui::Window *parent, TrackLayer *t, const Range &range, const string &text, const TrackMarker *marker);
	MarkerDialog(hui::Window *parent, TrackLayer *t, const Range &range, const string &text);
	MarkerDialog(hui::Window *parent, TrackLayer *t, const TrackMarker *marker);

	void on_edit();
	void on_ok();
	void on_close();

	enum class Mode {
		Text,
		Key
	} mode;

	TrackLayer *layer;
	Range range;
	string text;
	const TrackMarker *marker;
};

}

#endif /* SRC_VIEW_DIALOG_MARKERDIALOG_H_ */
