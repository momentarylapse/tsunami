/*
 * TrackHeightManager.h
 *
 *  Created on: 21.06.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_TRACKHEIGHTMANAGER_H_
#define SRC_VIEW_TRACKHEIGHTMANAGER_H_

#include "../../lib/math/rect.h"
#include "../../lib/os/time.h"

namespace tsunami {

class Song;
class AudioView;
class Track;

class TrackHeightManager {
public:
	TrackHeightManager();

	float t;
	bool _dirty;
	void set_dirty();
	bool animating;
	rect render_area;
	os::Timer timer;

	bool check(Song *s);
	bool update(AudioView *v, Song *s, const rect &r);
	void plan(AudioView *v, Song *s, const rect &r);
	void update_immediately(AudioView *v, Song *s, const rect &r);
};

}

#endif /* SRC_VIEW_TRACKHEIGHTMANAGER_H_ */
