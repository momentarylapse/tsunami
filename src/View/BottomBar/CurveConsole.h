/*
 * CurveConsole.h
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#ifndef CURVECONSOLE_H_
#define CURVECONSOLE_H_

#include "BottomBar.h"
#include "../../Stuff/Observer.h"

class AudioFile;
class AudioView;

class CurveConsole : public BottomBarConsole, public Observer
{
public:
	CurveConsole(AudioView *view, AudioFile *audio);
	virtual ~CurveConsole();


	virtual void OnUpdate(Observable *o, const string &message);

	AudioFile *audio;
	AudioView *view;
};

#endif /* CURVECONSOLE_H_ */
