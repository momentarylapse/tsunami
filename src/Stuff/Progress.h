/*
 * Progress.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include "../lib/hui/hui.h"
#include "Observable.h"

class Progress : public Observable
{
public:
	Progress();
	virtual ~Progress();
	void Set(float progress);
	void Set(const string &str, float progress);
	void Start(const string &str, float progress);
	void StartCancelable(const string &str, float progress);
	void End();

	void Cancel();
	bool IsCancelled();

private:
	CHuiWindow *dlg;
	bool Cancelled;
};

#endif /* PROGRESS_H_ */
