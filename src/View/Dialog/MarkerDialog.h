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

class Track;
class TrackMarker;

class MarkerDialog: public hui::Window
{
public:
	MarkerDialog(hui::Window *_parent, Track *t, const Range &range, const TrackMarker *marker);
	virtual ~MarkerDialog();

	void on_edit();
	void on_ok();
	void on_close();

	Track *track;
	Range range;
	const TrackMarker *marker;
};

#endif /* SRC_VIEW_DIALOG_MARKERDIALOG_H_ */
