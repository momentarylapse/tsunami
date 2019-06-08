/*
 * TrackHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_TRACKHEADER_H_
#define SRC_VIEW_NODE_TRACKHEADER_H_

#include "ViewNode.h"

class AudioViewTrack;



class TrackHeader : public ViewNode
{
public:
	AudioViewTrack *vtrack;
	TrackHeader(AudioViewTrack *t);
	void draw(Painter *c) override;

	bool on_left_button_down() override;
	bool on_right_button_down() override;
};



#endif /* SRC_VIEW_NODE_TRACKHEADER_H_ */
