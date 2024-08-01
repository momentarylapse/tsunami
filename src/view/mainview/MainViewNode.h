//
// Created by michi on 5/7/24.
//

#ifndef TSUNAMI_MAINVIEWNODE_H
#define TSUNAMI_MAINVIEWNODE_H

#include "../helper/graph/Node.h"

namespace tsunami {

class MainViewNode : public ::scenegraph::Node {
public:
	obs::source out_delete_me{this, "delete-me"};

	virtual void* mvn_data() const = 0;
	virtual string mvn_description() const = 0;
	virtual void mvn_on_enter() {}

	// TODO clean up this stupid interface!
	virtual void mvn_play() {}
	virtual void mvn_play_toggle() {}
	virtual void mvn_stop() {}
	virtual void mvn_pause() {}
	virtual bool mvn_is_playing() { return false; }
	virtual bool mvn_can_play() { return false; }
	virtual bool mvn_can_stop() { return false; }
	virtual bool mvn_can_pause() { return false; }
	virtual bool mvn_can_record() { return false; }
};

}

#endif //TSUNAMI_MAINVIEWNODE_H
