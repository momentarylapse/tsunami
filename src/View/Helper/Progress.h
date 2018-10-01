/*
 * Progress.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include "../../lib/hui/hui.h"
#include "../../Stuff/Observable.h"

class Progress : public Observable<VirtualBase>
{
public:
	Progress(const string &str, hui::Window *parent);
	virtual ~Progress();

	void _cdecl __init__(const string &str, hui::Window *parent);
	virtual void _cdecl __delete__();

	void set(float progress);
	void set(const string &str, float progress);

	void _cdecl set_kaba(const string &str, float progress);

	static const string MESSAGE_CANCEL;

	void _cdecl cancel();
	bool _cdecl is_cancelled();

protected:
	Progress();
	hui::Window *dlg;
	hui::Timer timer;
	float allow_next;
	bool cancelled;
};

class ProgressCancelable : public Progress
{
public:
	ProgressCancelable(const string &str, hui::Window *parent);
	ProgressCancelable();
	virtual ~ProgressCancelable();

	void _cdecl __init__(const string &str, hui::Window *parent);
	virtual void _cdecl __delete__();
};

#endif /* PROGRESS_H_ */
