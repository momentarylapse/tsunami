#include "Controls/ControlButton.h"
#include "Controls/ControlCheckBox.h"
#include "Controls/ControlColorButton.h"
#include "Controls/ControlComboBox.h"
#include "Controls/ControlDrawingArea.h"
#include "Controls/ControlEdit.h"
#include "Controls/ControlExpander.h"
#include "Controls/ControlGrid.h"
#include "Controls/ControlGroup.h"
#include "Controls/ControlLabel.h"
#include "Controls/ControlListView.h"
#include "Controls/ControlMultilineEdit.h"
#include "Controls/ControlPaned.h"
#include "Controls/ControlProgressBar.h"
#include "Controls/ControlRadioButton.h"
#include "Controls/ControlRevealer.h"
#include "Controls/ControlScroller.h"
#include "Controls/ControlSeparator.h"
#include "Controls/ControlSlider.h"
#include "Controls/ControlSpinButton.h"
#include "Controls/ControlTabControl.h"
#include "Controls/ControlToggleButton.h"
#include "Controls/ControlTreeView.h"
#include "hui.h"
#include "internal.h"
#ifdef HUI_API_GTK
#include "../math/math.h"

#ifndef OS_WINDOWS
#include <pango/pangocairo.h>
#endif

namespace hui
{

GtkTreeIter dummy_iter;

void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;


//----------------------------------------------------------------------------------
// creating control items


#if !GTK_CHECK_VERSION(2,22,0)
	void gtk_table_get_size(GtkTable *table, guint *rows, guint *columns)
	{
		g_object_get(G_OBJECT(table), "n-rows", rows, NULL);
		g_object_get(G_OBJECT(table), "n-columns", columns, NULL);
	}
#endif

#if !GTK_CHECK_VERSION(3,0,0)
	GtkWidget *gtk_scale_new_with_range(GtkOrientation orientation, double min, double max, double step)
	{
		if (orientation == GTK_ORIENTATION_VERTICAL)
			return gtk_vscale_new_with_range(min, max, step);
		else
			return gtk_hscale_new_with_range(min, max, step);
	}

	void gtk_combo_box_text_remove_all(GtkComboBoxText *c)
	{
		GtkTreeModel *m = gtk_combo_box_get_model(GTK_COMBO_BOX(c));
		int n = gtk_tree_model_iter_n_children(m, NULL);
		for (int i=0;i<n;i++)
			gtk_combo_box_text_remove(c, 0);
	}

	void gtk_combo_box_text_append(GtkComboBoxText *c, const char *id, const char *text)
	{
		gtk_combo_box_text_append_text(c, text);
	}

	GtkWidget *gtk_box_new(GtkOrientation orientation, int spacing)
	{
		if (orientation == GTK_ORIENTATION_VERTICAL)
			return gtk_vbox_new(false, spacing);
		else
			return gtk_hbox_new(false, spacing);
	}
#endif

void Panel::_insert_control_(Control *c, int x, int y, int width, int height)
{
	GtkWidget *frame = c->get_frame();
	c->panel = this;
	c->is_button_bar = false;
	if (is_resizable){
		if (cur_control){
			if (cur_control->type == CONTROL_GRID){
				cur_control->add(c, x, y);
			}else if (cur_control->type == CONTROL_TABCONTROL){
				cur_control->add(c, tab_creation_page, 0);
			}else if (cur_control->type == CONTROL_GROUP){
				cur_control->add(c, 0, 0);
			}else if (cur_control->type == CONTROL_EXPANDER){
				cur_control->add(c, 0, 0);
			}else if (cur_control->type == CONTROL_SCROLLER){
				cur_control->add(c, 0, 0);
			}else if (cur_control->type == CONTROL_REVEALER){
				cur_control->add(c, 0, 0);
			}
		}else{
			root_control = c;
			// directly into the window...
			//gtk_container_add(GTK_CONTAINER(plugable), frame);
			if (plugable){
				// this = HuiWindow...
				gtk_box_pack_start(GTK_BOX(plugable), frame, true, true, 0);
				gtk_container_set_border_width(GTK_CONTAINER(plugable), border_width);
			}
		}
	}else{
		if ((c->type == CONTROL_BUTTON) or (c->type == CONTROL_COLORBUTTON) or (c->type == CONTROL_COMBOBOX)){
			x -= 1;
			y -= 1;
			width += 2;
			height += 2;
		}else if (c->type == CONTROL_LABEL){
			y -= 4;
			height += 8;
		}else if (c->type == CONTROL_DRAWINGAREA){
			/*x -= 2;
			y -= 2;*/
		}
		// fixed
		GtkWidget *target_widget = plugable;
		if (cur_control)
			target_widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(cur_control->widget), tab_creation_page); // selected by SetTabCreationPage()...
			
		gtk_widget_set_size_request(frame, width, height);
		gtk_fixed_put(GTK_FIXED(target_widget), frame, x, y);
	}
	if (frame != c->widget)
		gtk_widget_show(frame);
	gtk_widget_show(c->widget);
	c->enabled = true;
	//c->TabID = TabCreationID;
	//c->TabPage = TabCreationPage;
	/*c->x = 0;
	c->y = 0;*/
	controls.add(c);
}

Control *Panel ::_get_control_(const string &id)
{
	if ((id.num == 0) and (cur_id.num > 0))
		return _get_control_(cur_id);

	// search backwards -> multiple AddText()s with identical ids
	//   will always set their own text
	foreachb(Control *c, controls)
		if (c->id == id)
			return c;

	if (id.num != 0){
		// ...test if exists in menu/toolbar before reporting an error!
		//msg_error("hui: unknown id: '" + id + "'");
	}
	return NULL;
}

Control *Panel::_get_control_by_widget_(GtkWidget *widget)
{
	for (int j=0;j<controls.num;j++)
		if (controls[j]->widget == widget)
			return controls[j];
	return NULL;
}

string Panel::_get_id_by_widget_(GtkWidget *widget)
{
	for (int j=0;j<controls.num;j++)
		if (controls[j]->widget == widget)
			return controls[j]->id;
	return "";
}



void NotifyWindowByWidget(Panel *panel, GtkWidget *widget, const string &message = "", bool is_default = true)
{
	if (allow_signal_level > 0)
		return;
	msg_db_m("NotifyWindowByWidget", 2);
	string id = panel->_get_id_by_widget_(widget);
	panel->_set_cur_id_(id);
	if (id.num > 0){
		Event e = Event(id, message);
		e.is_default = is_default;
		panel->_send_event_(&e);
	}
}

void SetImageById(Panel *panel, const string &id)
{
	if ((id == "ok") or (id == "cancel") or (id == "apply"))
		panel->setImage(id, "hui:" + id);
	else if (id != ""){
		for (auto &c: panel->event_listeners)
			if ((c.id == id) and (c.image != ""))
				panel->setImage(id, c.image);
	}
}



void Panel::addButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlButton(title, id), x, y, width, height);

	SetImageById(this, id);
}

void Panel::addColorButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlColorButton(title, id), x, y, width, height);
}

void Panel::addDefButton(const string &title,int x,int y,int width,int height,const string &id)
{
	addButton(title, x, y, width, height, id);
	GtkWidget *b = controls.back()->widget;
	gtk_widget_set_can_default(b, true);
	gtk_widget_grab_default(b);
}




void Panel::addCheckBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlCheckBox(title, id), x, y, width, height);
}

void Panel::addLabel(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlLabel(title, id), x, y, width, height);
}



void Panel::addEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlEdit(title, id), x, y, width, height);
}

void Panel::addMultilineEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlMultilineEdit(title, id), x, y, width, height);
	if (win)
		if ((!win->main_input_control) and ((ControlMultilineEdit*)controls.back())->handle_keys)
			win->main_input_control = controls.back();
}

void Panel::addSpinButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlSpinButton(title, id), x, y, width, height);
}

void Panel::addGroup(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlGroup(title, id), x, y, width, height);
}

void Panel::addComboBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlComboBox(title, id), x, y, width, height);
}

void Panel::addToggleButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlToggleButton(title, id), x, y, width, height);
}

void Panel::addRadioButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlRadioButton(title, id, this), x, y, width, height);
}

void Panel::addTabControl(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlTabControl(title, id, this), x, y, width, height);
}

void Panel::setTarget(const string &id,int page)
{
	tab_creation_page = page;
	cur_control = NULL;
	if (id.num > 0)
		for (int i=0;i<controls.num;i++)
			if (id == controls[i]->id)
				cur_control = controls[i];
}

void Panel::addListView(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlListView(title, id, this), x, y, width, height);
}

void Panel::addTreeView(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlTreeView(title, id, this), x, y, width, height);
}

void Panel::addIconView(const string &title,int x,int y,int width,int height,const string &id)
{
	msg_todo("AddIconView: deprecated");
	/*
	GetPartStrings(id,title);

	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	GtkWidget *view;

	GtkListStore *model;
	model = gtk_list_store_new (2, G_TYPE_STRING,GDK_TYPE_PIXBUF);
	view=gtk_icon_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(view),0);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(view),1);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(view),130);
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(view), GTK_SELECTION_SINGLE);
	// react on double click
	g_signal_connect(G_OBJECT(view), "item-activated", G_CALLBACK(&OnGtkIconListActivate), this);
	if (OptionString.find("nobar") >= 0)
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), false);

	GtkWidget *frame = gtk_frame_new(NULL);
	//gtk_widget_set_size_request(frame,width-4,height-4);
	//gtk_fixed_put(GTK_FIXED(cur_cnt),frame,x,y);
	//gtk_container_add(GTK_CONTAINER(frame),c.win);
	gtk_container_add(GTK_CONTAINER(frame),sw);
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);
	_InsertControl_(view, x, y, width, height, id, HuiKindIconView, frame);*/
}

void Panel::addProgressBar(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlProgressBar(title, id), x, y, width, height);
}

void Panel::addSlider(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlSlider(title, id, height > width), x, y, width, height);
}

void Panel::addImage(const string &title,int x,int y,int width,int height,const string &id)
{
	msg_todo("AddImage: deprecated");
	/*GetPartStrings(id, title);
	GtkWidget *im;
	if (PartString[0].num > 0){
		if (PartString[0][0] == '/')
			im = gtk_image_new_from_file(sys_str_f(PartString[0]));
		else
			im = gtk_image_new_from_file(sys_str_f(HuiAppDirectoryStatic + PartString[0]));
	}else
		im = gtk_image_new();
	_InsertControl_(im, x, y, width, height, id, HuiKindImage);*/
}


void Panel::addDrawingArea(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlDrawingArea(title, id), x, y, width, height);
	if (win and (!win->main_input_control))
		win->main_input_control = controls.back();
}


void Panel::addGrid(const string &title, int x, int y, int width, int height, const string &id)
{
	_insert_control_(new ControlGrid(title, id, width, height, this), x, y, width, height);
}

void Panel::addExpander(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlExpander(title, id), x, y, width, height);
}

void Panel::addPaned(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlPaned(title, id), x, y, width, height);
}

void Panel::addScroller(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlScroller(title, id), x, y, width, height);
}

void Panel::addSeparator(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlSeparator(title, id), x, y, width, height);
}

void Panel::addRevealer(const string &title,int x,int y,int width,int height,const string &id)
{
	_insert_control_(new ControlRevealer(title, id), x, y, width, height);
}

void Panel::embedDialog(const string &id, int x, int y)
{
	border_width = 8;

	Resource *res = GetResource(id);
	if (!res)
		return;
	if (res->type != "SizableDialog")
		return;
	if (res->children.num == 0)
		return;
	Resource rr = res->children[0];
	rr.x = x;
	rr.y = y;

	string parent_id;
	if (cur_control)
		parent_id = cur_control->id;
	_addControl(id, rr, parent_id);
}

void hui_rm_event(Array<EventListener> &event, Control *c)
{
	for (int i=0; i<event.num; i++)
		if (event[i].id == c->id){
			//msg_write("erase event");
			event.erase(i);
			i --;
		}
	for (Control *cc : c->children)
		hui_rm_event(event, cc);
}

void Panel::removeControl(const string &id)
{
	Control *c = _get_control_(id);
	if (c){
		hui_rm_event(event_listeners, c);
		delete(c);
	}
}



//----------------------------------------------------------------------------------
// drawing

void Panel::redraw(const string &_id)
{
	Control *c = _get_control_(_id);
	if (c){
		GdkWindow *w = gtk_widget_get_window(c->widget);
		if (w)
			gdk_window_invalidate_rect(w, NULL, false);
	}
}

void Panel::redrawRect(const string &_id, const rect &r)
{
	Control *c = _get_control_(_id);
	if (c){

		/*if (w < 0){
			x += w;
			w = - w;
		}
		if (h < 0){
			y += h;
			h = - h;
		}*/
		GdkRectangle rr;
		rr.x = r.x1;
		rr.y = r.y1;
		rr.width = r.width();
		rr.height = r.height();
		gdk_window_invalidate_rect(gtk_widget_get_window(c->widget), &rr, false);
	}
}

}


#endif
