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

namespace tsunami {

class CaptureConsoleModeMulti: public CaptureConsoleMode {
public:
	CaptureConsoleModeMulti(CaptureConsole *cc);
	void enter() override;
	void leave() override;
	void allow_change_device(bool allow) override;

	void on_source();
};

}

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_ */
