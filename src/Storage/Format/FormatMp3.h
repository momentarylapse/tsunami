/*
 * FormatMp3.h
 *
 *  Created on: 18.09.2014
 *      Author: michi
 */

#ifndef FORMATMP3_H_
#define FORMATMP3_H_

#include "Format.h"

class FormatMp3: public Format
{
public:
	FormatMp3();
	virtual ~FormatMp3();

	void loadTrack(StorageOperationData *od);
	void saveBuffer(StorageOperationData *od);

	void loadSong(StorageOperationData *od);
	void saveSong(StorageOperationData *od);
};

#endif /* FORMATMP3_H_ */
