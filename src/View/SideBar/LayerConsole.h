/*
 * LayerConsole.h
 *
 *  Created on: 23.05.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_LAYERCONSOLE_H_
#define SRC_VIEW_SIDEBAR_LAYERCONSOLE_H_

#include "SideBar.h"

class LayerConsole: public SideBarConsole
{
public:
	LayerConsole(Session *session);
	virtual ~LayerConsole();

	void loadData();
	void applyData();

	void onSelect();
	void onEdit();
	void onMove();
	void onAdd();
	void onDelete();
	void onMerge();

	void onEditSong();

	void onUpdate();
};

#endif /* SRC_VIEW_SIDEBAR_LAYERCONSOLE_H_ */
