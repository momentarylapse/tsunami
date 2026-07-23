/*
 * ActionSampleAdd.h
 *
 *  Created on: 15.07.2013
 *      Author: michi
 */

#ifndef ACTIONSAMPLEADD_H_
#define ACTIONSAMPLEADD_H_

#include <lib/history/Action.h>

namespace tsunami {

struct AudioBuffer;
struct MidiNoteBuffer;
struct Sample;

class ActionSampleAdd : public history::Action {
public:
	explicit ActionSampleAdd(shared<Sample> s);

	string name() const override { return ":##:add sample"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	shared<Sample> sample;
};

}

#endif /* ACTIONSAMPLEADD_H_ */
