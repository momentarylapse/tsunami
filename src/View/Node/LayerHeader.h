/*
 * LayerHeader.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_LAYERHEADER_H_
#define SRC_VIEW_NODE_LAYERHEADER_H_

#include "ViewNode.h"

class AudioViewLayer;


class LayerHeader : public ViewNode
{
public:
	AudioViewLayer *vlayer;
	LayerHeader(AudioViewLayer *l);
	void draw(Painter *c) override;
};



#endif /* SRC_VIEW_NODE_LAYERHEADER_H_ */
