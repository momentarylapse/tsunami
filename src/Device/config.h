/*
 * config.h
 *
 *  Created on: 29.05.2016
 *      Author: michi
 */


#ifndef SRC_DEVICE_CONFIG_H_
#define SRC_DEVICE_CONFIG_H_


#include "../lib/base/macros.h"

#ifdef OS_LINUX

#define DEVICE_PULSEAUDIO 1
//#define DEVICE_PORTAUDIO  1

#define DEVICE_MIDI_ALSA 1

#endif



#ifdef OS_WINDOWS

#define DEVICE_PORTAUDIO 1

#endif



#endif /* SRC_DEVICE_CONFIG_H_ */
