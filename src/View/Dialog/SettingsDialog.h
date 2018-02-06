/*
 * SettingsDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include "../../lib/hui/hui.h"

class AudioView;

class SettingsDialog: public hui::ResourceWindow
{
public:
	SettingsDialog(AudioView *view, hui::Window *parent);

	void loadData();
	void applyData();

	void onLanguage();
	void onColorScheme();
	void onOggBitrate();
	void onDefaultArtist();
	void onCaptureFilename();
	void onCaptureFind();


private:
	struct OggQuality
	{
		OggQuality(){}
		OggQuality(float q, int b) : quality(q), bitrate(b){};
		float quality;
		int bitrate;
	};

	Array<OggQuality> ogg_quality;
	AudioView *view;
};

#endif /* SETTINGSDIALOG_H_ */
