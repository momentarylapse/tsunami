/*
 * ViewModeEdit.cpp
 *
 *  Created on: May 1, 2020
 *      Author: michi
 */

#include "ViewModeEdit.h"
#include "ViewModeEditDummy.h"
#include "ViewModeMidi.h"
#include "ViewModeEditAudio.h"
#include "../AudioView.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../Graph/AudioViewLayer.h"

ViewModeEdit::ViewModeEdit(AudioView *view) : ViewModeDefault(view) {
	mode = view->mode_edit_midi;
}

ViewModeEdit::~ViewModeEdit() {
}

void ViewModeEdit::on_start() {
	set_mode(suggest_mode());
	mode->on_start();
}

void ViewModeEdit::on_end() {
	mode->on_end();
}

void ViewModeEdit::on_key_down(int k) {

	if (k == hui::KEY_UP + hui::KEY_ALT)
		view->move_to_layer(-1);
	if (k == hui::KEY_DOWN + hui::KEY_ALT)
		view->move_to_layer(1);

	mode->on_key_down(k);
}

void ViewModeEdit::on_command(const string &id) {
	mode->on_command(id);
}

float ViewModeEdit::layer_suggested_height(AudioViewLayer *l) {
	return mode->layer_suggested_height(l);
}

void ViewModeEdit::set_mode(ViewMode *m) {
	if (mode == m)
		return;
	if (view->mode == this)
		mode->on_end();
	mode = m;
	if (view->mode == this)
		mode->on_start();
	notify();
}

void ViewModeEdit::on_cur_layer_change() {
	view->thm.set_dirty();
	set_mode(suggest_mode());
	mode->on_cur_layer_change();
}

ViewMode *ViewModeEdit::suggest_mode() {
	if (view->cur_track()->type == SignalType::MIDI)
		return view->mode_edit_midi;
	if (view->cur_track()->type == SignalType::AUDIO)
		return view->mode_edit_audio;
	return view->mode_edit_dummy;
}


bool ViewModeEdit::editing(AudioViewLayer *l) {
	if (view->mode != this)
		return false;
	if (l != view->cur_vlayer())
		return false;
	return true;
}

void ViewModeEdit::draw_layer_background(Painter *c, AudioViewLayer *l) {
	mode->draw_layer_background(c, l);
}

void ViewModeEdit::draw_post(Painter *c) {
	mode->draw_post(c);

	ViewModeDefault::draw_selected_layer_highlight(c, view->cur_vlayer()->area);
}

HoverData ViewModeEdit::get_hover_data(AudioViewLayer *vlayer, const vec2 &m) {
	return mode->get_hover_data(vlayer, m);
}

SongSelection ViewModeEdit::get_selection_for_rect(const Range &r, int y0, int y1) {
	return mode->get_selection_for_rect(r, y0, y1);
}

SongSelection ViewModeEdit::get_selection_for_range(const Range &r) {
	return mode->get_selection_for_range(r);
}

void ViewModeEdit::left_click_handle_void(AudioViewLayer *vlayer) {
	if (vlayer == view->cur_vlayer()) {
		mode->left_click_handle_void(vlayer);
	} else {
		ViewModeDefault::left_click_handle_void(vlayer);
	}
	view->exclusively_select_layer(vlayer);
}

string ViewModeEdit::get_tip() {
	return mode->get_tip();
}
