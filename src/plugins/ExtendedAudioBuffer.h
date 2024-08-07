/*
 * ExtendedAudioBuffer.h
 *
 *  Created on: 01.02.2013
 *      Author: michi
 */

#ifndef SRC_PLUGINS_EXTENDEDAUDIOBUFFER_H_
#define SRC_PLUGINS_EXTENDEDAUDIOBUFFER_H_

#include "../data/audio/AudioBuffer.h"
class complex;

namespace tsunami {

class ExtendedAudioBuffer : public AudioBuffer {
public:

	void get_spectrum(Array<complex> &spec_r, Array<complex> &spec_l, int samples);
};

}

#endif /* SRC_PLUGINS_EXTENDEDAUDIOBUFFER_H_ */
