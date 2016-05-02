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
	SettingsDialog(HuiWindow *parent);
	virtual ~SettingsDialog();

	void loadData();
	void applyData();

	void onLanguage();
	void onColorScheme();
	void onOggBitrate();
	void onDefaultArtist();
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
};

#endif /* SETTINGSDIALOG_H_ */
