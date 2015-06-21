/*
 * TrackHeightManager.h
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_TRACKHEIGHTMANAGER_H_
#define SRC_VIEW_TRACKHEIGHTMANAGER_H_

#include "../lib/hui/hui.h"
#include "../lib/math/rect.h"

class AudioFile;
class AudioView;
class Track;

class TrackHeightManager
{
public:
	TrackHeightManager();

	float t;
	bool dirty;
	bool animating;
	rect render_area;
	HuiTimer timer;
	Track *midi_track;

	bool check(AudioFile *a);
	bool update(AudioView *v, AudioFile *a, const rect &r);
	void plan(AudioView *v, AudioFile *a, const rect &r);
};

#endif /* SRC_VIEW_TRACKHEIGHTMANAGER_H_ */
