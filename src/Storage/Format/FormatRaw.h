/*
 * FormatRaw.h
 *
 *  Created on: 16.02.2013
 *      Author: michi
 */

#ifndef FORMATRAW_H_
#define FORMATRAW_H_

#include "Format.h"

class FormatRaw: public Format
{
public:
	FormatRaw();
	virtual ~FormatRaw();

	void loadTrack(StorageOperationData *od);
	void saveBuffer(StorageOperationData *od);

	void loadAudio(StorageOperationData *od);
	void saveAudio(StorageOperationData *od);
};

#endif /* FORMATRAW_H_ */
