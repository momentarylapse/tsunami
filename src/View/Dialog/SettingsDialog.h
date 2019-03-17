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

class SettingsDialog: public hui::Window
{
public:
	SettingsDialog(AudioView *view, hui::Window *parent);

	void load_data();
	void applyData();

	void on_language();
	void on_color_scheme();
	void on_ogg_bitrate();
	void on_default_artist();
	void on_scroll_speed();
	void on_audio_api();
	void on_midi_api();
	void on_prebuffer();
	void on_suck_buffer();
	void on_cpu_meter();
	void on_antialiasing();
	void on_high_details();
	void on_qed_find();


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
