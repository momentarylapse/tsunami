/*
 * FormatOgg.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATOGG_H_
#define FORMATOGG_H_

#include "Format.h"

class FormatOgg: public Format
{
public:
	FormatOgg();
	virtual ~FormatOgg();

	void loadTrack(StorageOperationData *od);
	void saveBuffer(StorageOperationData *od);

	void loadAudio(StorageOperationData *od);
	void saveAudio(StorageOperationData *od);
};

#endif /* FORMATOGG_H_ */
