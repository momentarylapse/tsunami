/*
 * ActionBarEdit.cpp
 *
 *  Created on: 15.12.2012
 *      Author: michi
 */

#include "ActionBarEdit.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../data/Song.h"
#include "../../data/rhythm/Bar.h"
#include "../../processing/audio/BufferInterpolator.h"
#include "Action__ScaleData.h"
#include "ActionBar__Edit.h"
#include "../track/buffer/ActionTrackEditBuffer.h"
#include "../track/buffer/ActionTrack__GrowBuffer.h"
#include "../track/buffer/ActionTrack__ShrinkBuffer.h"
#include <assert.h>

namespace tsunami {

ActionBarEdit::ActionBarEdit(int _index, const BarPattern &b, BarEditMode _mode) {
	index = _index;
	length = b.length;
	divisor = b.divisor;
	beats = b.beats;
	mode = _mode;
}

void ActionBarEdit::build(Data *d) {
	Song *s = dynamic_cast<Song*>(d);
	Range r = Range(s->bar_offset(index), s->bars[index]->length);

	float scale_factor = (float)length / (float)r.length;

	add_sub_action(new ActionBar__Edit(index, length, beats, divisor), d);
	if (mode != BarEditMode::Ignore) {
		bool scale_audio = (mode == BarEditMode::StretchAndScaleAudio);

		if (scale_audio) {
			for (auto t: weak(s->tracks))
				for (auto l: weak(t->layers))
					foreachi (AudioBuffer &b, l->buffers, index)
						if (b.range().overlaps(r)){
							Range stretch_zone = b.range().intersect(r);
							int new_stretch_size = (int)((float)stretch_zone.length * scale_factor);
							int new_buffer_size = b.length + new_stretch_size - stretch_zone.length;

							// save the part after the stretch
							AudioBuffer b_after_stretch;
							b_after_stretch.resize(b.range().end() - stretch_zone.end());
							b_after_stretch.set(b, b.offset - stretch_zone.end(), 1.0f);

							// grow?
							if (scale_factor > 1.0f)
								add_sub_action(new ActionTrack__GrowBuffer(l, index, new_buffer_size), d);

							// stretch input
							AudioBuffer b_stretch_in;
							int stretch_offset = stretch_zone.offset - b.offset;
							b_stretch_in.set_as_ref(b, stretch_offset, stretch_zone.length);

							Range rr = Range::to(stretch_zone.start(), b.range().end());
							auto *a = new ActionTrackEditBuffer(l, rr);

							// stretch and insert
							AudioBuffer b_stretch_out; // use ref...
							b_stretch_out.resize(new_stretch_size);
							BufferInterpolator::interpolate(b_stretch_in, b_stretch_out, BufferInterpolator::Method::Linear);
							b.set(b_stretch_out, stretch_offset, 1.0f);

							// re-insert after stretch
							b.set(b_after_stretch, new_buffer_size - b_after_stretch.length, 1.0f);

							add_sub_action(a, d);


							// shrink?
							if (scale_factor < 1.0f)
								add_sub_action(new ActionTrack__ShrinkBuffer(l, index, new_buffer_size), d);

						}
		}


		add_sub_action(new Action__ScaleData(r, length), d);
	}
}

}

