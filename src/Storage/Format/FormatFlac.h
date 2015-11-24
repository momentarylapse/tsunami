/*
 * FormatFlac.h
 *
 *  Created on: 06.04.2012
 *      Author: michi
 */

#ifndef FORMATFLAC_H_
#define FORMATFLAC_H_

#include "Format.h"

class FormatFlac: public Format
{
public:
	FormatFlac();
	virtual ~FormatFlac();

	virtual void loadTrack(StorageOperationData *od);
	virtual void saveViaRenderer(StorageOperationData *od);
};

#endif /* FORMATFLAC_H_ */
