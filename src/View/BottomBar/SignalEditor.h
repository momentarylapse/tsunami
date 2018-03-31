/*
 * SignalEditor.h
 *
 *  Created on: 30.03.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_
#define SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_


#include "BottomBar.h"

class DeviceManager;
class Device;
class Painter;
class AudioSource;
class MidiSource;

class SignalEditor: public BottomBar::Console
{
public:
	SignalEditor(Session *session);
	virtual ~SignalEditor();

	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMouseMove();
	void onDraw(Painter *p);

	enum ModuleType{
		TYPE_SONG_RENDERER,
		TYPE_AUDIO_SUCKER,
		TYPE_OUTPUT_STREAM,
		TYPE_INPUT_STREAM_AUDIO,
		TYPE_INPUT_STREAM_MIDI,
		TYPE_PEAK_METER,
		TYPE_AUDIO_EFFECT,
		TYPE_MIDI_EFFECT,
		TYPE_SYNTHESIZER,
	};

	class Module
	{
	public:
		virtual ~Module(){}
		float x, y;
		virtual string type() = 0;
		virtual void set_audio_source(AudioSource *s){};
		virtual void set_midi_source(MidiSource *s){};
		virtual AudioSource *get_audio_source(){ return NULL; }
		virtual MidiSource *get_midi_source(){ return NULL; }
	};
	Array<Module*> modules;

	struct Cable
	{
		int type;
		Module *source, *target;
	};
	Array<Cable*> cables;
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALEDITOR_H_ */
