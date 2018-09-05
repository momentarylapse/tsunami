/*
 * ActionTrackLayerMarkDominant.cpp
 *
 *  Created on: 04.09.2018
 *      Author: michi
 */

#include "ActionTrackLayerMarkDominant.h"
#include "../../../Data/Track.h"

ActionTrackLayerMarkDominant::ActionTrackLayerMarkDominant(TrackLayer *layer, const Range &range)
{
	track = layer->track;
	int index = layer->version_number();

	int index_before = 0;
	int index_after = 0;

	// delete fades in range
	foreachi (auto &f, track->fades, i){
		if (f.position < range.start())
			index_before = f.target;
		if (f.position < range.end())
			index_after = f.target;
		if (range.is_inside(f.position)){
			Operation op;
			op.index = i;
			deletes.add(op);
		}
	}

	// add new fades
	Operation op;
	op.position = range.start();
	op.samples = 2000; // for now, use default value
	op.target = index;
	if (index != index_before)
		inserts.add(op);
	op.position = range.end();
	op.samples = 2000;
	op.target = index_after;
	if (index != index_after)
		inserts.add(op);
}

void* ActionTrackLayerMarkDominant::execute(Data* d)
{
	foreachb(auto &op, deletes){
		//printf("del #%d at %d\n", op.index, op.samples);
		op.position = track->fades[op.index].position;
		op.samples = track->fades[op.index].samples;
		op.target = track->fades[op.index].target;
		track->fades.erase(op.index);
	}

	for (auto &op: inserts){
		Track::Fade f;
		f.position = op.position;
		f.samples = op.samples;
		f.target = op.target;
		op.index = 0;
		for (int i=0; i<track->fades.num; i++)
			if (track->fades[i].position < f.position){
				op.index = i + 1;
			}
		//printf("add #%d at %d\n", op.index, op.samples);
		track->fades.insert(f, op.index);
	}

	/*printf("------\n");
	for (auto &f: track->fades)
		printf("  %d %d\n", f.position, f.target);*/
	return 0;
}

void ActionTrackLayerMarkDominant::undo(Data* d)
{
	foreachb(auto &op, inserts){
		//printf("del #%d at %d\n", op.index, op.samples);
		op.position = track->fades[op.index].position;
		op.samples = track->fades[op.index].samples;
		op.target = track->fades[op.index].target;
		track->fades.erase(op.index);
	}

	for (auto &op: deletes){
		Track::Fade f;
		f.position = op.position;
		f.samples = op.samples;
		f.target = op.target;
		op.index = 0;
		for (int i=0; i<track->fades.num; i++)
			if (track->fades[i].position < f.position){
				op.index = i + 1;
			}
		//printf("add #%d at %d\n", op.index, op.samples);
		track->fades.insert(f, op.index);
	}

	/*printf("------\n");
	for (auto &f: track->fades)
		printf("  %d %d\n", f.position, f.target);*/
}
