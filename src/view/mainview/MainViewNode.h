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

	virtual void play() {}
	virtual void stop() {}
	virtual void pause() {}
	virtual bool is_playback_active() { return false; } // playing or paused
	virtual bool is_paused() { return false; }
	virtual bool is_recording() { return false; }
	virtual bool is_editing() { return false; }
	virtual bool mvn_can_play() { return false; }
	virtual bool mvn_can_stop() { return false; }
	virtual bool mvn_can_pause() { return false; }
	virtual bool mvn_can_record() { return false; }
	virtual bool mvn_can_edit() { return false; }
	virtual bool mvn_show_undo() { return false; }
};

}

#endif //TSUNAMI_MAINVIEWNODE_H
