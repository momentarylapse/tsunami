/*
 * MarkerDialog.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "MarkerDialog.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/midi/Scale.h"

namespace tsunami {

string encode_key(const Scale &key) {
	return "::key=" + key.encode() + "::";
}

MarkerDialog::MarkerDialog(hui::Window* _parent, TrackLayer* _l, const Range &_range, const string &_text, const TrackMarker *_marker):
	hui::Dialog("marker_dialog", _parent)
{
	layer = _l;
	range = _range;
	text = _text;
	marker = _marker; // might be nil
	mode = Mode::Text;
	if (marker_is_key(text)) {
		mode = Mode::Key;
	}

	for (int i=0; i<12; i++)
		add_string("key_root", rel_pitch_name(11 - i));
	for (int i=0; i<(int)Scale::Type::Count; i++)
		add_string("key_type", Scale::get_type_name((Scale::Type)i));
	auto key = parse_marker_key(text);
	set_int("key_root", 11 - key.root);
	set_int("key_type", (int)key.type);

	set_string("text", text);
	enable("ok", text.num > 0);
	hide_control("grid_text", mode != Mode::Text);
	hide_control("grid_key", mode != Mode::Key);

	event("text", [this] { on_edit(); });
	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
	event("ok", [this] { on_ok(); });
}

MarkerDialog::MarkerDialog(hui::Window* _parent, TrackLayer* _l, const Range &_range, const string &_text):
	MarkerDialog(_parent, _l, _range, _text, nullptr) {}

MarkerDialog::MarkerDialog(hui::Window* _parent, TrackLayer* _l, const TrackMarker *_marker):
	MarkerDialog(_parent, _l, _marker->range, _marker->text, _marker) {}

void MarkerDialog::on_edit() {
	enable("ok", get_string("text").num > 0);
}

void MarkerDialog::on_ok() {
	string text = get_string("text");
	if (mode == Mode::Key) {
		Scale key = Scale((Scale::Type)get_int("key_type"), 11 - get_int("key_root"));
		text = encode_key(key);
	}

	if (marker) {
		layer->edit_marker(marker, marker->range, text);
	} else {
		layer->add_marker(new TrackMarker(range, text));
	}
	request_destroy();
}

void MarkerDialog::on_close() {
	request_destroy();
}

}
