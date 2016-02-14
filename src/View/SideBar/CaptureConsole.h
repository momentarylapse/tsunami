/*
 * CaptureDialog.h
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#ifndef CAPTURECONSOLE_H_
#define CAPTURECONSOLE_H_


#include "SideBar.h"
#include "../Helper/PeakMeter.h"
#include "../../Stuff/Observer.h"
#include "../../Audio/AudioInputMidi.h"
#include "../../Data/Song.h"

class AudioView;
class AudioInputAny;

class CaptureConsole : public SideBarConsole, public Observer
{
public:
	CaptureConsole(Song *s, AudioView *view);
	virtual ~CaptureConsole();



	virtual void onEnter();
	virtual void onLeave();

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
	Array<AudioInputMidi::MidiPort> midi_ports;
	void updateAudioSourceList();
	Array<string> audio_sources;
	string selected_audio_source;
	void updateTime();

	void onUpdate(Observable *o, const string &message);

	void setTarget(int index);
	void setType(int type);

	Song *song;
	AudioView *view;
	AudioInputAny *input;
	PeakMeter *peak_meter;
	Synthesizer *temp_synth;
	int type;
};

#endif /* CAPTURECONSOLE_H_ */
