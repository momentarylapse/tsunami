/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef CAPTUREDIALOG_H_
#define CAPTUREDIALOG_H_


#include "../../lib/hui/hui.h"
#include "../../Data/AudioFile.h"
#include "../Helper/PeakMeter.h"
#include "../../Stuff/Observer.h"
#include "../../Audio/AudioInputMidi.h"

class AudioView;
class AudioInput;

class CaptureDialog : public HuiWindow, public Observer
{
public:
	CaptureDialog(HuiWindow *_parent, bool _allow_parent, AudioFile *a);
	virtual ~CaptureDialog();

	void onTypeAudio();
	void onTypeMidi();
	void onSource();
	void onTarget();
	void onStart();
	void onDelete();
	void onPause();
	void onOk();
	void onClose();
	bool insert();

	void fillTrackList();

	void updateMidiPortList();
	Array<AudioInput::MidiPort> midi_ports;
	void updateAudioSourceList();
	Array<string> audio_sources;
	string selected_audio_source;

	void onUpdate(Observable *o, const string &message);

	void setTarget(int index);
	void setType(int type);

	AudioFile *audio;
	AudioView *view;
	AudioInput *input;
	PeakMeter *peak_meter;
	Synthesizer *temp_synth;
	int type;
};

#endif /* CAPTUREDIALOG_H_ */
