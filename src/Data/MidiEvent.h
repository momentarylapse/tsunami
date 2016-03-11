/*
 * MidiEvent.h
 *
 *  Created on: 09.03.2016
 *      Author: michi
 */

#ifndef SRC_DATA_MIDIEVENT_H_
#define SRC_DATA_MIDIEVENT_H_



class MidiEvent
{
public:
	MidiEvent(){}
	MidiEvent(int pos, float pitch, float volume);
	int pos;
	float pitch;
	float volume;
};



#endif /* SRC_DATA_MIDIEVENT_H_ */
