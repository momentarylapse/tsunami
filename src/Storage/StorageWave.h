/*
 * StorageWave.h
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#ifndef STORAGEWAVE_H_
#define STORAGEWAVE_H_

#include "StorageAny.h"

class StorageWave: public StorageAny
{
public:
	StorageWave();
	virtual ~StorageWave();

	void LoadTrack(Track *t, const string &filename);
	void SaveTrack(Track *t, const string &filename);
};

#endif /* STORAGEWAVE_H_ */
