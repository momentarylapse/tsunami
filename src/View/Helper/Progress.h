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

class Progress : public Observable
{
public:
	Progress(const string &str, hui::Window *parent);
	virtual ~Progress();
	void set(float progress);
	void set(const string &str, float progress);

	static const string MESSAGE_CANCEL;

	void cancel();
	bool isCancelled();

protected:
	Progress();
	hui::Window *dlg;
	bool cancelled;
};

class ProgressCancelable : public Progress
{
public:
	ProgressCancelable(const string &str, hui::Window *parent);
	virtual ~ProgressCancelable();
};

#endif /* PROGRESS_H_ */
