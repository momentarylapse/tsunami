/*
 * Progress.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include "../../lib/base/pointer.h"
#include "../../lib/os/time.h"
#include "../../lib/pattern/Observable.h"

namespace hui {
	class Window;
}

namespace tsunami {

class Progress : public obs::Node<VirtualBase> {
public:
	Progress(const string &str, hui::Window *parent);
	~Progress() override {};

	void _cdecl __init__(const string &str, hui::Window *parent);
	void _cdecl __delete__() override;

	void set(float progress);
	void set(const string &str, float progress);

	void _cdecl set_kaba(const string &str, float progress);

	obs::source out_cancel{this, "cancel"};

	void _cdecl cancel();
	bool _cdecl is_cancelled();

protected:
	Progress();
	owned<hui::Window> dlg;
	owned<os::Timer> timer;
	float allow_next;
	bool cancelled;
};

class ProgressCancelable : public Progress {
public:
	ProgressCancelable(const string &str, hui::Window *parent);
	ProgressCancelable();
	~ProgressCancelable() override {};

	void _cdecl __init__(const string &str, hui::Window *parent);
	void _cdecl __delete__() override;
};

}

#endif /* PROGRESS_H_ */
