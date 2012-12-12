/*
 * Plugin.h
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include "../lib/base/strings.h"
#include "../Data/AudioFile.h"

class CScript;
class sType;

// represents a compiled script
class Plugin
{
public:
	typedef void void_func();
	typedef void process_track_func(BufferBox*, Track*, int);

	Plugin(const string &_filename);

	string filename;
	CScript *s;
	void_func *f_reset;
	void_func *f_data2dialog;
	void_func *f_configure;
	void_func *f_reset_state;
	process_track_func *f_process_track;
	int index;
	sType *state_type;
	void *state;
	sType *data_type;
	void *data;

	bool usable;
	int type;
	enum{
		TYPE_EFFECT,
		TYPE_OTHER
	};

	void ExportData(Array<EffectParam> &param);
	void ImportData(Array<EffectParam> &param);
	void ResetData();
	void ResetState();
	bool Configure(bool previewable);
	void DataToDialog();
	void ProcessTrack(Track *t, int level_no, Range r);

	void WriteDataToFile(const string &name);
	void LoadDataFromFile(const string &name);

	string GetError();
};

#endif /* PLUGIN_H_ */
