/*
 * MarkerDialog.cpp
 *
 *  Created on: May 13, 2015
 *      Author: ankele
 */

#include "MarkerDialog.h"
#include "../../Data/Track.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/Midi/Scale.h"

bool marker_is_key(const string &text)
{
	return text.match("::key=*::");
}

Scale parse_marker_key(const string &text)
{
	Scale scale = Scale::C_MAJOR;
	if (!marker_is_key(text))
		return scale;
	return Scale::parse(text.substr(6, -3));
}

string encode_key(const Scale &key)
{
	return "::key=" + key.encode() + "::";
}

MarkerDialog::MarkerDialog(hui::Window* _parent, Track* _t, const Range &_range, const string &_text, const TrackMarker *_marker):
	hui::Window("marker_dialog", _parent)
{
	track = _t;
	range = _range;
	text = _text;
	marker = _marker; // might be nil
	mode = Mode::TEXT;
	if (marker_is_key(text)){
		mode = Mode::KEY;
	}

	for (int i=0; i<12; i++)
		add_string("key_root", rel_pitch_name(11 - i));
	for (int i=0; i<(int)Scale::Type::NUM_TYPES; i++)
		add_string("key_type", Scale::get_type_name((Scale::Type)i));
	auto key = parse_marker_key(text);
	set_int("key_root", 11 - key.root);
	set_int("key_type", (int)key.type);

	set_string("text", text);
	enable("ok", text.num > 0);
	hide_control("grid_text", mode != Mode::TEXT);
	hide_control("grid_key", mode != Mode::KEY);

	event("text", [=]{ on_edit(); });
	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
	event("ok", [=]{ on_ok(); });
}

MarkerDialog::MarkerDialog(hui::Window* _parent, Track* _t, const Range &_range, const string &_text):
	MarkerDialog(_parent, _t, _range, _text, nullptr){}

MarkerDialog::MarkerDialog(hui::Window* _parent, Track* _t, const TrackMarker *_marker):
	MarkerDialog(_parent, _t, _marker->range, _marker->text, _marker){}

MarkerDialog::~MarkerDialog()
{
}

void MarkerDialog::on_edit()
{
	enable("ok", get_string("text").num > 0);
}

void MarkerDialog::on_ok()
{
	string text = get_string("text");
	if (mode == Mode::KEY){
		Scale key = Scale((Scale::Type)get_int("key_type"), 11 - get_int("key_root"));
		text = encode_key(key);
	}

	if (marker){
		track->edit_marker(marker, marker->range, text);
	}else{
		track->add_marker(range, text);
	}
	destroy();
}

void MarkerDialog::on_close()
{
	destroy();
}
