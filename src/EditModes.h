/*
 * EditModes.h
 *
 *  Created on: Sep 30, 2020
 *      Author: michi
 */

#ifndef SRC_EDITMODES_H_
#define SRC_EDITMODES_H_

#include "lib/base/base.h"

namespace tsunami {

class EditMode {
public:
	static const string Default;
	static const string DefaultSong;
	static const string DefaultTrack;
	static const string DefaultTrackFx;
	static const string DefaultTrackSynth;
	static const string DefaultTrackMidiFx;
	static const string DefaultFx;
	static const string DefaultMidiFx;
	static const string DefaultSamples;
	static const string DefaultMixing;
	static const string DefaultMastering;
	static const string DefaultSampleRef;
	static const string XSignalEditor;
	static const string EditTrack;
	static const string Capture;
	static const string ScaleBars;
	static const string ScaleMarker;
	static const string Curves;
};

}


#endif /* SRC_EDITMODES_H_ */
