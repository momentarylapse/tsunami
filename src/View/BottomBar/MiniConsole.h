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
class AudioOutput;

class MiniConsole : public HuiPanel
{
public:
	MiniConsole(AudioOutput *output);
	virtual ~MiniConsole();

	virtual void OnShow();
	virtual void OnHide();

	void OnShowFxConsole();
	void OnShowMixingConsole();

	AudioOutput *output;
	PeakMeter *peak_meter;
};

#endif /* MINICONSOLE_H_ */
