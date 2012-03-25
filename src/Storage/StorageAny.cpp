/*
 * StorageAny.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "StorageAny.h"

StorageAny::StorageAny(const string &_extension)
{
	extension = _extension;
}

StorageAny::~StorageAny()
{
}

void StorageAny::ImportData(Track *t, void *data, int channels, int bits, int samples)
{
	msg_db_r("ImportData", 1);

	/*char *cb = (char*)data;
	short *sb = (short*)data;
	BufferBox buf = t->GetBuffers(0, samples);

	for (int i=0;i<samples;i++){
		if (channels == 2){
			if (bits == 8){
				buf.r[i] = (float)cb[i*2] / 128.0f;
				buf.l[i] = (float)cb[i*2+1] / 128.0f;
			}else if (bits == 16){
				buf.r[i] = (float)sb[i*2] / 32768.0f;
				buf.l[i] = (float)sb[i*2+1] / 32768.0f;
			}else if (bits == 24){
				buf.r[i] = (float)*(short*)&cb[i*6 + 1] / 32768.0f; // only high 16 bits
				buf.l[i] = (float)*(short*)&cb[i*6 + 4] / 32768.0f;
			}else{ // 32
				buf.r[i] = (float)sb[i*4+1] / 32768.0f; // only high 16 bits...
				buf.l[i] = (float)sb[i*4+3] / 32768.0f;
			}
		}else{
			if (bits == 8){
				buf.r[i] = (float)cb[i] / 128.0f;
			}else if (bits == 16){
				buf.r[i] = (float)sb[i] / 32768.0f;
			}else if (bits == 24){
				buf.r[i] = (float)*(short*)&cb[i*3 + 1] / 32768.0f;
			}else{ // 32
				buf.r[i] = (float)sb[i*2+1] / 32768.0f;
			}
			buf.l[i] = buf.r[i];
		}
	//	if ((i & 65535) == 0)
	//		ProgressStatus(_("importiere Daten"), perc_import + dperc_import * (float)i / (float)samples);
	}*/
	msg_db_l(1);
}


bool StorageAny::CanHandle(const string & _extension)
{
	return (extension == _extension);
}

