/*
 * SettingsDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include "../../lib/hui/hui.h"

class SettingsDialog: public HuiWindow
{
public:
	SettingsDialog(HuiWindow *_parent, bool _allow_parent);
	virtual ~SettingsDialog();

	void loadData();
	void applyData();

	void onLanguage();
	void onColorScheme();
	void onOggBitrate();
	void onDefaultArtist();
	void onPreviewDevice();
	void onCaptureDevice();
	void onCaptureDelay();
	void onCaptureFilename();
	void onCaptureFind();
	void onClose();


private:
	struct OggQuality
	{
		OggQuality(){}
		OggQuality(float q, int b) : quality(q), bitrate(b){};
		float quality;
		int bitrate;
	};

	Array<OggQuality> ogg_quality;
	Array<string> output_devices;
	Array<string> capture_devices;
};

#endif /* SETTINGSDIALOG_H_ */
