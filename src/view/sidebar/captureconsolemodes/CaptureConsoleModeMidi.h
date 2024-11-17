/*
 * CaptureConsoleModeMidi.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_

#include "CaptureConsoleMode.h"
#include "../../../lib/base/base.h"

namespace tsunami {

class CaptureConsoleModeMidi : public CaptureConsoleMode {
public:
	explicit CaptureConsoleModeMidi(CaptureConsole *cc);

	void enter() override;
	void allow_change_device(bool allow) override;
};

}

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_ */
