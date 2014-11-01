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
	Progress();
	virtual ~Progress();
	void set(float progress);
	void set(const string &str, float progress);
	void start(const string &str, float progress);
	void startCancelable(const string &str, float progress);
	void end();

	static const string MESSAGE_CANCEL;

	void cancel();
	bool isCancelled();

private:
	HuiWindow *dlg;
	bool Cancelled;
};

#endif /* PROGRESS_H_ */
