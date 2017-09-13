/*
 * RawConfigDialog.h
 *
 *  Created on: 07.07.2013
 *      Author: michi
 */

#ifndef RAWCONFIGDIALOG_H_
#define RAWCONFIGDIALOG_H_

#include "../../Data/AudioBuffer.h"
#include "../../lib/hui/hui.h"

struct RawConfigData
{
	SampleFormat format;
	int channels;
	int sample_rate;
	int offset;
};

class RawConfigDialog : public hui::Window
{
public:
	RawConfigDialog(RawConfigData *data, hui::Window *parent);
	virtual ~RawConfigDialog();

	void onClose();
	void onOk();

	RawConfigData *data;
	bool ok;
};

#endif /* RAWCONFIGDIALOG_H_ */
