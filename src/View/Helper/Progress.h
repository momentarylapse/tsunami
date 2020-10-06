/*
 * Progress.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include "../../lib/base/pointer.h"
#include "../../Stuff/Observable.h"

namespace hui {
	class Window;
	class Timer;
}

class Progress : public Observable<VirtualBase> {
public:
	Progress(const string &str, hui::Window *parent);
	~Progress() override {};

	void _cdecl __init__(const string &str, hui::Window *parent);
	void _cdecl __delete__() override;

	void set(float progress);
	void set(const string &str, float progress);

	void _cdecl set_kaba(const string &str, float progress);

	static const string MESSAGE_CANCEL;

	void _cdecl cancel();
	bool _cdecl is_cancelled();

protected:
	Progress();
	owned<hui::Window> dlg;
	owned<hui::Timer> timer;
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

#endif /* PROGRESS_H_ */
