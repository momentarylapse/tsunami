/*
 * LayerConsole.cpp
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */


#include "LayerConsole.h"

#include "../../Tsunami.h"
#include "../../View/AudioView.h"
#include "../../Data/Song.h"
#include "../../Stuff/Log.h"


LayerConsole::LayerConsole(Song *s, AudioView *v) :
	SideBarConsole(_("Layers"))
{
	song = s;
	view = v;

	// dialog
	setBorderWidth(5);
	embedDialog("layer_dialog", 0, 0);
	setDecimals(1);

	loadData();

	eventX("layers", "hui:select", std::bind(&LayerConsole::onSelect, this));
	eventX("layers", "hui:change", std::bind(&LayerConsole::onEdit, this));
	eventX("layers", "hui:move", std::bind(&LayerConsole::onMove, this));
	event("add_layer", std::bind(&LayerConsole::onAdd, this));
	event("delete_layer", std::bind(&LayerConsole::onDelete, this));
	event("merge_layer", std::bind(&LayerConsole::onMerge, this));

	event("edit_song", std::bind(&LayerConsole::onEditSong, this));

	song->subscribe_old(this, LayerConsole);
	view->subscribe_old2(this, LayerConsole, view->MESSAGE_CUR_LAYER_CHANGE);
}

LayerConsole::~LayerConsole()
{
	song->unsubscribe(this);
	view->unsubscribe(this);
}

void LayerConsole::loadData()
{
	reset("layers");
	foreachi(Song::Layer *l, song->layers, i)
		addString("layers", i2s(i + 1) + "\\" + l->name + "\\" + b2s(l->active));
	if (song->layers.num > 0)
		setInt("layers", view->cur_layer);

	enable("delete_layer", song->layers.num > 1);
	enable("merge_layer", view->cur_layer > 0);
}


void LayerConsole::onSelect()
{
	int s = getInt("layers");
	view->setCurLayer(s);
}

void LayerConsole::onEdit()
{
	int row = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (row < 0)
		return;
	if (col == 1){
		song->renameLayer(row, getCell("layers", row, 1));
	}else if (col == 2){
		song->layers[row]->active = getCell("layers", row, 2)._bool();
		loadData();
	}
}

void LayerConsole::onMove()
{
	int source = hui::GetEvent()->row;
	int target = hui::GetEvent()->row_target;
	if ((source < 0) or (target < 0))
		return;
	song->moveLayer(source, target);
	view->setCurLayer(target);
}

void LayerConsole::onAdd()
{
	int cur_layer = view->cur_layer;
	song->addLayer("", cur_layer + 1);
	view->setCurLayer(cur_layer + 1);
}

void LayerConsole::onDelete()
{
	try{
		song->deleteLayer(view->cur_layer);
	}catch(Song::Exception &e){
		tsunami->log->error(e.message);
	}
}

void LayerConsole::onMerge()
{
	int s = getInt("layers");
	if (s >= 1)
		song->mergeLayers(s, s - 1);
}

void LayerConsole::onEditSong()
{
	bar()->open(SideBar::SONG_CONSOLE);
}

void LayerConsole::onUpdate(Observable *o)
{
	loadData();
}
