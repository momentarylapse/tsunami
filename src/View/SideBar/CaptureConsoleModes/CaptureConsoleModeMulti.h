/*
 * CaptureConsoleModeMulti.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_

#include "CaptureConsoleMode.h"
#include "../../../lib/base/base.h"

class Device;
class Track;

class CaptureConsoleModeMulti: public CaptureConsoleMode
{
	Array<Device*> sources_audio;
	Array<Device*> sources_midi;

	struct CaptureItem
	{
		Track *track;
		Device *device;
		string id_source, id_target, id_type;
	};
	Array<CaptureItem> items;


public:
	CaptureConsoleModeMulti(CaptureConsole *cc);
	virtual ~CaptureConsoleModeMulti();
	//virtual void enter_parent();
	//virtual void leave_parent();
	void enter() override;
	void leave() override;
	void pause() override {}
	void start() override {}
	void stop() override {}
	void dump() override {}
	bool insert() override { return false; }
	int get_sample_count() override { return 0; }
	bool is_capturing() override { return false; }
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_ */
