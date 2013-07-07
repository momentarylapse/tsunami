#include "hui.h"
#include "hui_internal.h"
#include "Controls/HuiControlButton.h"
#include "Controls/HuiControlCheckBox.h"
#include "Controls/HuiControlColorButton.h"
#include "Controls/HuiControlComboBox.h"
#include "Controls/HuiControlDrawingArea.h"
#include "Controls/HuiControlEdit.h"
#include "Controls/HuiControlGrid.h"
#include "Controls/HuiControlGroup.h"
#include "Controls/HuiControlLabel.h"
#include "Controls/HuiControlListView.h"
#include "Controls/HuiControlMultilineEdit.h"
#include "Controls/HuiControlProgressBar.h"
#include "Controls/HuiControlRadioButton.h"
#include "Controls/HuiControlSlider.h"
#include "Controls/HuiControlSpinButton.h"
#include "Controls/HuiControlTabControl.h"
#include "Controls/HuiControlToggleButton.h"
#include "Controls/HuiControlTreeView.h"
#ifdef HUI_API_GTK
#include "../math/math.h"

#ifndef OS_WINDOWS
#include <pango/pangocairo.h>
#endif

GtkTreeIter dummy_iter;

void GetPartStrings(const string &id, const string &title);
//string ScanOptions(int id, const string &title);
extern Array<string> PartString;
extern string OptionString, HuiFormatString;


//----------------------------------------------------------------------------------
// creating control items



enum{
	HuiGtkInsertContainer,
	HuiGtkInsertTable,
	HuiGtkInsertTabControl,
};

#if GTK_MAJOR_VERSION == 2
#if GTK_MINOR_VERSION < 22
	void gtk_table_get_size(GtkTable *table, guint *rows, guint *columns)
	{
		g_object_get(G_OBJECT(table), "n-rows", rows, NULL);
		g_object_get(G_OBJECT(table), "n-columns", columns, NULL);
	}
#endif

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

void HuiWindow::_InsertControl_(HuiControl *c, int x, int y, int width, int height)
{
	GtkWidget *frame = c->frame;
	if (!frame)
		frame = c->widget;
	c->win = this;
	c->is_button_bar = false;
	if (is_resizable){
		HuiControl *root_ctrl = cur_control;
		GtkWidget *target_widget = plugable;
		int root_type = HuiGtkInsertContainer;
		if (root_ctrl){
			if (root_ctrl->type == HuiKindControlTable){
				root_type = HuiGtkInsertTable;
				target_widget = root_ctrl->widget;
				unsigned int n_cols, n_rows;
				gtk_table_get_size(GTK_TABLE(target_widget), &n_rows, &n_cols);

				// top table?
				if (cur_control == control[0])
					if ((n_cols == 1) && (y == (n_rows - 1)))
						c->is_button_bar = true;
			}else if (root_ctrl->type == HuiKindTabControl){
				root_type = HuiGtkInsertTabControl;
				target_widget = root_ctrl->widget;
				target_widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(target_widget), tab_creation_page); // selected by SetTabCreationPage()...
			}else
				root_type = -1;
		}
		bool root_is_button_bar = false;
		if (root_ctrl)
			if (root_ctrl->is_button_bar)
				root_is_button_bar = true;
		// insert
		if (root_type == HuiGtkInsertContainer){
			// directly into the window...
			//gtk_container_add(GTK_CONTAINER(target_widget), frame);
			gtk_box_pack_start(GTK_BOX(target_widget), frame, true, true, 0);
	//		_cur_widget_ = frame;
			/*if ((kind == HuiKindListView) || (kind == HuiKindDrawingArea) || (kind == HuiKindMultilineEdit))
				gtk_container_set_border_width(GTK_CONTAINER(target_widget), 0);
			else*/
				gtk_container_set_border_width(GTK_CONTAINER(target_widget), border_width);
		}else if (root_type == HuiGtkInsertTable){
			GtkAttachOptions op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			GtkAttachOptions op_y = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			if (c->type == HuiKindButton){
				//gtk_widget_set_size_request(frame, 100, 30);
				const char *_label = gtk_button_get_label(GTK_BUTTON(c->widget));
				if ((_label) && (strlen(_label) > 0)){ // != NULL ... cause set even if ""
					if (root_is_button_bar){
						op_x = GtkAttachOptions(GTK_FILL);
						//gtk_widget_set_size_request(frame, 120, 30);
						gtk_widget_set_size_request(frame, -1, 30);
					}else{
						op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
						gtk_widget_set_size_request(frame, -1, 30);
					}
				}else{
					op_x = GtkAttachOptions(GTK_FILL);
					gtk_widget_set_size_request(frame, 30, 30);
				}
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
			/*}else if ((type == HuiKindEdit) || (kind == HuiKindComboBox)  || (type == HuiKindCheckBox) || (type == HuiKindControlTable)){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);*/
			}else if (c->type == HuiKindColorButton){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, 100, 28);
			}else if (c->type == HuiKindComboBox){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, -1, 28);
			}else if ((c->type == HuiKindEdit) || (c->type == HuiKindSpinButton) || (c->type == HuiKindCheckBox) || (c->type == HuiKindRadioButton) || (c->type == HuiKindSlider) || (c->type == HuiKindProgressBar)){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, -1, 25);
			}else if (c->type == HuiKindControlTable){
				//op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
			}else if (c->type == HuiKindText){
				//op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
				if (!root_is_button_bar)
					op_x = GtkAttachOptions(GTK_FILL);
				op_y = GtkAttachOptions(GTK_FILL);
			}
			if (OptionString.find("noexpandy") >= 0)
				op_y = GtkAttachOptions(GTK_FILL);
			else if (OptionString.find("expandy") >= 0)
				op_y = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			if (OptionString.find("noexpandx") >= 0)
				op_x = GtkAttachOptions(GTK_FILL);
			else if (OptionString.find("expandx") >= 0)
				op_x = GtkAttachOptions(GTK_FILL | GTK_EXPAND);
			if (OptionString.find("width") >= 0){
				string ww = OptionString.substr(OptionString.find("width") + 6, -1);
				if (ww.find(","))
					ww = ww.substr(0, ww.find(","));
				int width = s2i(ww);
				gtk_widget_set_size_request(frame, width, 28);
				op_x = GtkAttachOptions(0);
			}

			// TODO
			unsigned int nx, ny;
			gtk_table_get_size(GTK_TABLE(target_widget), &ny, &nx);
			if (x >= nx){
				y += (x / nx);
				x = (x % nx);
			}

			
			gtk_table_attach(GTK_TABLE(target_widget), frame, x, x+1, y, y+1, op_x, op_y, 0, 0);
			if (root_is_button_bar)
				gtk_container_child_set(GTK_CONTAINER(target_widget), frame, "y-padding", 7, NULL);
		}else if (root_type == HuiGtkInsertTabControl){
			gtk_container_add(GTK_CONTAINER(target_widget), frame);
			gtk_container_set_border_width(GTK_CONTAINER(target_widget), border_width);
		}
	}else{
		if ((c->type == HuiKindButton) || (c->type == HuiKindColorButton) || (c->type == HuiKindComboBox)){
			x -= 1;
			y -= 1;
			width += 2;
			height += 2;
		}else if (c->type == HuiKindText){
			y -= 4;
			height += 8;
		}else if (c->type == HuiKindDrawingArea){
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
	control.add(c);
}

HuiControl *HuiWindow ::_GetControl_(const string &id)
{
	if ((id.num == 0) && (cur_id.num > 0))
		return _GetControl_(cur_id);

	// search backwards -> multiple AddText()s with identical ids
	//   will always set their own text
	foreachb(HuiControl *c, control)
		if (c->id == id)
			return c;

	if (id.num != 0){
		// ...test if exists in menu/toolbar before reporting an error!
		//msg_error("hui: unknown id: '" + id + "'");
	}
	return NULL;
}

HuiControl *HuiWindow::_GetControlByWidget_(GtkWidget *widget)
{
	for (int j=0;j<control.num;j++)
		if (control[j]->widget == widget)
			return control[j];
	return NULL;
}

string HuiWindow::_GetIDByWidget_(GtkWidget *widget)
{
	for (int j=0;j<control.num;j++)
		if (control[j]->widget == widget)
			return control[j]->id;
	return "";
}



void NotifyWindowByWidget(HuiWindow *win, GtkWidget *widget, const string &message = "", bool is_default = true)
{
	if (allow_signal_level > 0)
		return;
	msg_db_m("NotifyWindowByWidget", 2);
	string id = win->_GetIDByWidget_(widget);
	win->_SetCurID_(id);
	if (id.num > 0){
		HuiEvent e = HuiEvent(id, message);
		_HuiSendGlobalCommand_(&e);
		e.is_default = is_default;
		win->_SendEvent_(&e);
	}
}

void SetImageById(HuiWindow *win, const string &id)
{
	if ((id == "ok") || (id == "cancel") || (id == "apply"))
		win->SetImage(id, "hui:" + id);
	else if (id != ""){
		foreach(HuiCommand &c, _HuiCommand_)
			if ((c.id == id) && (c.image != ""))
				win->SetImage(id, c.image);
	}
}



void HuiWindow::AddButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlButton(title, id), x, y, width, height);

	SetImageById(this, id);
}

void HuiWindow::AddColorButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlColorButton(title, id), x, y, width, height);
}

void HuiWindow::AddDefButton(const string &title,int x,int y,int width,int height,const string &id)
{
	AddButton(title, x, y, width, height, id);
	GtkWidget *b = control.back()->widget;
#ifdef OS_WINDOWS
	GTK_WIDGET_SET_FLAGS(b, GTK_CAN_DEFAULT);
#else
	gtk_widget_set_can_default(b, true);
#endif
	gtk_widget_grab_default(b);
}




void HuiWindow::AddCheckBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlCheckBox(title, id), x, y, width, height);
}

void HuiWindow::AddText(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlLabel(title, id), x, y, width, height);
}



void HuiWindow::AddEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlEdit(title, id), x, y, width, height);

	// dumb but usefull test
/*	if (height > 30){
		GtkStyle* style = gtk_widget_get_style(text);
		PangoFontDescription *font_desc = pango_font_description_copy(style->font_desc);
		pango_font_description_set_absolute_size(font_desc, height * PANGO_SCALE * 0.95);
		gtk_widget_modify_font(text, font_desc);
	}*/
}

void HuiWindow::AddMultilineEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlMultilineEdit(title, id), x, y, width, height);
}

void HuiWindow::AddSpinButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlSpinButton(title, id), x, y, width, height);
}

void HuiWindow::AddGroup(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlGroup(title, id), x, y, width, height);
}

void HuiWindow::AddComboBox(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlComboBox(title, id), x, y, width, height);
}

void HuiWindow::AddToggleButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlToggleButton(title, id), x, y, width, height);
}

void HuiWindow::AddRadioButton(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlRadioButton(title, id, this), x, y, width, height);
}

void HuiWindow::AddTabControl(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlTabControl(title, id, this), x, y, width, height);
}

void HuiWindow::SetTarget(const string &id,int page)
{
	tab_creation_page = page;
	cur_control = NULL;
	if (id.num > 0)
		for (int i=0;i<control.num;i++)
			if (id == control[i]->id)
				cur_control = control[i];
}

void HuiWindow::AddListView(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlListView(title, id, this), x, y, width, height);
}

void HuiWindow::AddTreeView(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlTreeView(title, id, this), x, y, width, height);
}

void HuiWindow::AddIconView(const string &title,int x,int y,int width,int height,const string &id)
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

void HuiWindow::AddProgressBar(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlProgressBar(title, id), x, y, width, height);
}

void HuiWindow::AddSlider(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlSlider(title, id, height > width), x, y, width, height);
}

void HuiWindow::AddImage(const string &title,int x,int y,int width,int height,const string &id)
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


void HuiWindow::AddDrawingArea(const string &title,int x,int y,int width,int height,const string &id)
{
	_InsertControl_(new HuiControlDrawingArea(title, id), x, y, width, height);
	input_widget = control.back()->widget;
}


void HuiWindow::AddControlTable(const string &title, int x, int y, int width, int height, const string &id)
{
	_InsertControl_(new HuiControlGrid(title, id, width, height, this), x, y, width, height);
}

void HuiWindow::EmbedDialog(const string &id, int x, int y)
{
#if 0
	HuiWindow *dlg = HuiCreateResourceDialog(id, NULL, NULL);
	dlg->Update();

	for (int i=0;i<dlg->control.num;i++)
		control.add(dlg->control[i]);

	gtk_widget_unparent(dlg->control[0]->widget);

	//GtkWidget *b = gtk_button_new_with_label("test");

	
	_InsertControl_(this, dlg->control[0]->widget, root, page, 10, 10, 86237, dlg->control[0]->type);
	//_InsertControl_(this, b, root, page, 10, 10, 86237, dlg->control[0]->type);

	/*GtkWidget *p = gtk_widget_get_parent(b);
	gtk_widget_unparent(b);
	gtk_widget_reparent(dlg->control[0]->widget, p);*/
#endif

	border_width = 8;

	HuiResource *res = HuiGetResource(id);
	if (res){
		if (res->type != "SizableDialog")
			return;
		foreachi(HuiResourceCommand &cmd, res->cmd, i){
			//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
			//if ((cmd->type & 1023)==HuiCmdDialogAddControl){

			string target_id = cmd.s_param[0];
			int target_page = cmd.i_param[4];
			if (i > 0)
				SetTarget(target_id, target_page);
			int _x = (i == 0) ? x : cmd.i_param[0];
			int _y = (i == 0) ? y : cmd.i_param[1];
			HuiWindowAddControl( this, cmd.type, HuiGetLanguage(cmd.id),
								_x, _y,
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
			Enable(cmd.id, cmd.enabled);
			if (cmd.image.num > 0)
				SetImage(cmd.id, cmd.image);
		}
	}
}

void HuiWindow::RemoveControl(const string &id)
{
	HuiControl *c = _GetControl_(id);
	if (c)
		delete(c);
}



//----------------------------------------------------------------------------------
// drawing

void HuiWindow::Redraw(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (c)
		gdk_window_invalidate_rect(gtk_widget_get_window(c->widget), NULL, false);
}

void HuiWindow::RedrawRect(const string &_id, int x, int y, int w, int h)
{
	HuiControl *c = _GetControl_(_id);
	if (c){
		if (w < 0){
			x += w;
			w = - w;
		}
		if (h < 0){
			y += h;
			h = - h;
		}
		GdkRectangle r;
		r.x = x;
		r.y = y;
		r.width = w;
		r.height = h;
		gdk_window_invalidate_rect(gtk_widget_get_window(c->widget), &r, false);
	}
}


static HuiPainter hui_drawing_context;

HuiPainter *HuiWindow::BeginDraw(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	hui_drawing_context.win = this;
	hui_drawing_context.id = _id;
	hui_drawing_context.cr = NULL;
	if (c){
		hui_drawing_context.cr = gdk_cairo_create(gtk_widget_get_window(c->widget));
		//gdk_drawable_get_size(gtk_widget_get_window(c->widget), &hui_drawing_context.width, &hui_drawing_context.height);
		hui_drawing_context.width = gdk_window_get_width(gtk_widget_get_window(c->widget));
		hui_drawing_context.height = gdk_window_get_height(gtk_widget_get_window(c->widget));
		//hui_drawing_context.SetFontSize(16);
		hui_drawing_context.SetFont("Sans", 16, false, false);
	}
	return &hui_drawing_context;
}



#endif
