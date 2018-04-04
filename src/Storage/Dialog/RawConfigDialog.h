/*
 * RawConfigDialog.h
 *
 *  Created on: 07.07.2013
 *      Author: michi
 */

#ifndef SRC_STORAGE_DIALOG_RAWCONFIGDIALOG_H_
#define SRC_STORAGE_DIALOG_RAWCONFIGDIALOG_H_

#include "../../Data/Audio/AudioBuffer.h"
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

#endif /* SRC_STORAGE_DIALOG_RAWCONFIGDIALOG_H_ */
