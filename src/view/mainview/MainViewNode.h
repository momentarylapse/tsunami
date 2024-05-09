//
// Created by michi on 5/7/24.
//

#ifndef TSUNAMI_MAINVIEWNODE_H
#define TSUNAMI_MAINVIEWNODE_H

#include "../helper/graph/Node.h"

class MainViewNode : public scenegraph::Node {
public:
	obs::source out_delete_me{this, "delete-me"};

	virtual void* main_view_data() const = 0;
	virtual string main_view_description() const = 0;
	virtual void on_enter_main_view() {}
};


#endif //TSUNAMI_MAINVIEWNODE_H
