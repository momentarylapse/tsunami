/*
 * MiniConsole.h
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#ifndef MINICONSOLE_H_
#define MINICONSOLE_H_

#include "../../lib/hui/hui.h"

class PeakMeter;
class AudioStream;

class MiniConsole : public HuiPanel
{
public:
	MiniConsole(AudioStream *stream);
	virtual ~MiniConsole();

	virtual void onShow();
	virtual void onHide();

	void onShowFxConsole();
	void onShowMixingConsole();

	AudioStream *stream;
	PeakMeter *peak_meter;
};

#endif /* MINICONSOLE_H_ */
