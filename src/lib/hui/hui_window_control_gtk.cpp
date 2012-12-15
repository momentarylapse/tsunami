#include "hui.h"
#include "hui_internal.h"
#ifdef HUI_API_GTK

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

HuiControl *CHuiWindow::_InsertControl_(GtkWidget *widget, int x, int y, int width, int height, const string &id, int type, GtkWidget *frame)
{
	if (!frame)
		frame = widget;
	HuiControl *c = new HuiControl;
	c->win = this;
	c->widget = widget;
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
			if (type == HuiKindButton){
				//gtk_widget_set_size_request(frame, 100, 30);
				const char *_label = gtk_button_get_label(GTK_BUTTON(widget));
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
			}else if (type == HuiKindColorButton){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, 100, 28);
			}else if (type == HuiKindComboBox){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, -1, 28);
			}else if ((type == HuiKindEdit) || (type == HuiKindSpinButton) || (type == HuiKindCheckBox) || (type == HuiKindRadioButton) || (type == HuiKindSlider) || (type == HuiKindProgressBar)){
				op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
				gtk_widget_set_size_request(frame, -1, 25);
			}else if (type == HuiKindControlTable){
				//op_y = GtkAttachOptions(GTK_FILL);// | GTK_SHRINK);
			}else if (type == HuiKindText){
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
		if ((type == HuiKindButton) || (type == HuiKindColorButton) || (type == HuiKindComboBox)){
			x -= 1;
			y -= 1;
			width += 2;
			height += 2;
		}else if (type == HuiKindText){
			y -= 4;
			height += 8;
		}else if (type == HuiKindDrawingArea){
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
	if (frame != widget)
		gtk_widget_show(frame);
	gtk_widget_show(widget);
	c->id = id;
	c->type = type;
	c->enabled = true;
	//c->TabID = TabCreationID;
	//c->TabPage = TabCreationPage;
	/*c->x = 0;
	c->y = 0;*/
	control.add(c);
	return c;
}

HuiControl *CHuiWindow ::_GetControl_(const string &id)
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

HuiControl *CHuiWindow::_GetControlByWidget_(GtkWidget *widget)
{
	for (int j=0;j<control.num;j++)
		if (control[j]->widget == widget)
			return control[j];
	return NULL;
}

string CHuiWindow::_GetIDByWidget_(GtkWidget *widget)
{
	// controls
	for (int j=0;j<control.num;j++)
		if (control[j]->widget == widget)
			return control[j]->id;

	// toolbars
	for (int t=0;t<4;t++)
		for (int j=0;j<toolbar[t].item.num;j++)
			if ((GtkWidget*)toolbar[t].item[j].widget == widget)
				return toolbar[t].item[j].id;
	return "";
}



void NotifyWindowByWidget(CHuiWindow *win, GtkWidget *widget, const string &message = "", bool is_default = true)
{
	if (allow_signal_level > 0)
		return;
	msg_db_m("NotifyWindowByWidget", 2);
	string id = win->_GetIDByWidget_(widget);
	win->_SetCurID_(id);
	if (id.num > 0){
		HuiEvent e = HuiCreateEvent(id, message);
		_HuiSendGlobalCommand_(&e);
		e.is_default = is_default;
		win->_SendEvent_(&e);
	}
}

void SetImageById(CHuiWindow *win, const string &id)
{
	if ((id == "ok") || (id == "cancel") || (id == "apply"))
		win->SetImage(id, "hui:" + id);
	else if (id != "")
		foreach(HuiCommand &c, _HuiCommand_)
			if ((c.id == id) && (c.image != ""))
				win->SetImage(id, c.image);
}


void OnGtkButtonPress(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:click");	}

void CHuiWindow::AddButton(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *b = gtk_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
	_InsertControl_(b, x, y, width, height, id, HuiKindButton);

	SetImageById(this, id);
}

void CHuiWindow::AddColorButton(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *b = gtk_color_button_new();
	if (OptionString.find("alpha") >= 0)
		gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(b), true);
	g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
	_InsertControl_(b, x, y, width, height, id, HuiKindColorButton);
}

void CHuiWindow::AddDefButton(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *b = gtk_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(b),"clicked", G_CALLBACK(&OnGtkButtonPress), this);
	_InsertControl_(b, x, y, width, height, id, HuiKindButton);
#ifdef OS_WINDOWS
	GTK_WIDGET_SET_FLAGS(b, GTK_CAN_DEFAULT);
#else
	gtk_widget_set_can_default(b, true);
#endif
	gtk_widget_grab_default(b);

	SetImageById(this, id);
}



void OnGtkCheckboxChange(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");	}

void CHuiWindow::AddCheckBox(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *cb = gtk_check_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(cb), "clicked", G_CALLBACK(&OnGtkCheckboxChange), this);
	_InsertControl_(cb, x, y, width, height, id, HuiKindCheckBox);
}

void CHuiWindow::AddText(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *l = gtk_label_new("");
	if (OptionString.find("wrap") >= 0)
		gtk_label_set_line_wrap(GTK_LABEL(l),true);
	if (OptionString.find("center") >= 0)
		gtk_misc_set_alignment(GTK_MISC(l), 0.5f, 0.5f);
	else if (OptionString.find("right") >= 0)
		gtk_misc_set_alignment(GTK_MISC(l), 1, 0.5f);
	else
		gtk_misc_set_alignment(GTK_MISC(l), 0, 0.5f);
	string _id = (id.num > 0) ? id : "text_" + i2s(randi(10000));
	_InsertControl_(l, x, y, width, height, _id, HuiKindText);
	SetString(_id, title);
}



void OnGtkEditChange(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");	}

void CHuiWindow::AddEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *text = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(text), 3);
	gtk_entry_set_text(GTK_ENTRY(text), sys_str(PartString[0]));
	gtk_entry_set_activates_default(GTK_ENTRY(text), true);
	g_signal_connect(G_OBJECT(text), "changed", G_CALLBACK(&OnGtkEditChange), this);
	_InsertControl_(text, x, y, width, height, id, HuiKindEdit);

	// dumb but usefull test
	if (height > 30){
		GtkStyle* style = gtk_widget_get_style(text);
		PangoFontDescription *font_desc = pango_font_description_copy(style->font_desc);
		pango_font_description_set_absolute_size(font_desc, height * PANGO_SCALE * 0.95);
		gtk_widget_modify_font(text, font_desc);
	}
}

void CHuiWindow::AddMultilineEdit(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkTextBuffer *tb = gtk_text_buffer_new(NULL);
	GtkWidget *text = gtk_text_view_new_with_buffer(tb);
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scroll);
	gtk_container_add(GTK_CONTAINER(scroll), text);
	
	// frame
	GtkWidget *frame = scroll;
	if (border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), scroll);
	}
	g_signal_connect(G_OBJECT(tb), "changed", G_CALLBACK(&OnGtkEditChange), this);
	_InsertControl_(text, x, y, width, height, id, HuiKindMultilineEdit, frame);
	
	/*gtk_widget_set_size_request(frame,width,height);
	gtk_fixed_put(GTK_FIXED(cur_cnt),frame,x,y);
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(frame),scroll);
	gtk_widget_show(c.win);*/
}

void CHuiWindow::AddSpinButton(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	float vmin = -100000000000.0f;
	float vmax = 100000000000.0f;
	float step = 1;
	if (PartString.num >= 2)
		vmin = s2f(PartString[1]);
	if (PartString.num >= 3)
		vmax = s2f(PartString[2]);
	if (PartString.num >= 4)
		step = s2f(PartString[3]);
	GtkWidget *sb = gtk_spin_button_new_with_range(vmin, vmax, step);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb), s2f(PartString[0]));
	gtk_entry_set_activates_default(GTK_ENTRY(sb), true);
	g_signal_connect(G_OBJECT(sb), "changed", G_CALLBACK(&OnGtkEditChange), this);
	_InsertControl_(sb, x, y, width, height, id, HuiKindSpinButton);
}

void CHuiWindow::AddGroup(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *f = gtk_frame_new(sys_str(PartString[0]));
	_InsertControl_(f, x, y, width, height, id, HuiKindGroup);
}



void OnGtkComboboxChange(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");	}

void CHuiWindow::AddComboBox(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *cb = gtk_combo_box_text_new();
	g_signal_connect(G_OBJECT(cb), "changed", G_CALLBACK(&OnGtkComboboxChange), this);
	_InsertControl_(cb, x, y, width, height, id, HuiKindComboBox);
	if ((PartString.num > 1) || (PartString[0] != ""))
		for (int i=0;i<PartString.num;i++)
			AddString(id, PartString[i]);
	SetInt(id, 0);
}


void OnGtkToggleButtonToggle(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");	}

void CHuiWindow::AddToggleButton(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *cb = gtk_toggle_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(cb), "toggled", G_CALLBACK(&OnGtkToggleButtonToggle), this);
	_InsertControl_(cb, x, y, width, height, id, HuiKindToggleButton);
	SetInt(id, 0);
}


void OnGtkRadioButtonToggle(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");	}

void CHuiWindow::AddRadioButton(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	string group_id = id.substr(0, id.find(":"));
	GSList *group = NULL;
	foreach(HuiControl *c, control)
		if (c->type == HuiKindRadioButton)
			if (c->id.find(":"))
				if (c->id.substr(0, c->id.find(":")) == group_id)
					group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(c->widget));
		
	GtkWidget *cb = gtk_radio_button_new_with_label(group, sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(cb), "toggled", G_CALLBACK(&OnGtkRadioButtonToggle), this);
	_InsertControl_(cb, x, y, width, height, id, HuiKindRadioButton);
	SetInt(id, 0);
}




void OnGtkTabControlSwitch(GtkWidget *widget, GtkWidget *page, guint page_num, gpointer data)
{
	CHuiWindow *win = (CHuiWindow *)data;
	HuiControl *c = win->_GetControlByWidget_(widget);
	if (c)
		c->selected = page_num;
	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");
}

void CHuiWindow::AddTabControl(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *nb = gtk_notebook_new();
	HuiControl *c = _InsertControl_(nb, x, y, width, height, id, HuiKindTabControl);

	for (int i=0;i<PartString.num;i++){
		GtkWidget *inside;
		if (is_resizable){
			inside = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_box_set_homogeneous(GTK_BOX(inside), true);
		}else
			inside = gtk_fixed_new();
		gtk_widget_show(inside);
		GtkWidget *label = gtk_label_new(sys_str(PartString[i]));
		gtk_notebook_append_page(GTK_NOTEBOOK(nb), inside, label);
    }
	c->selected = 0;
	g_signal_connect(G_OBJECT(nb), "switch-page", G_CALLBACK(&OnGtkTabControlSwitch), this);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(nb), true);
	if (OptionString.find("nobar") >= 0)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(nb), false);
	//SetControlSelection(id, 0);
}

void CHuiWindow::SetTarget(const string &id,int page)
{
	tab_creation_page = page;
	cur_control = NULL;
	if (id.num > 0)
		for (int i=0;i<control.num;i++)
			if (id == control[i]->id)
				cur_control = control[i];
}

static void list_toggle_callback(GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
	HuiControl *c = (HuiControl*)data;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (cell), "column"));
	gtk_tree_model_get_iter(model, &iter, path);
	bool state;
	gtk_tree_model_get(model, &iter, column, &state, -1);
	state = !state;
	if (c->type == HuiKindListView)
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, state, -1);
	else if (c->type == HuiKindTreeView)
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, column, state, -1);

	c->win->input.column = column;
	c->win->input.row = s2i(path_string);
	NotifyWindowByWidget(c->win, c->widget, "hui:change", false);
	gtk_tree_path_free(path);
}


static void list_edited_callback(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data)
{
	HuiControl *c = (HuiControl*)data;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (cell), "column"));
	gtk_tree_model_get_iter(model, &iter, path);
	if (c->type == HuiKindListView)
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, new_text, -1);
	else if (c->type == HuiKindTreeView)
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, column, new_text, -1);
	

	c->win->input.column = column;
	c->win->input.row = s2i(path_string);
	NotifyWindowByWidget(c->win, c->widget, "hui:change", false);
	gtk_tree_path_free(path);
}

static GType HuiTypeList[64];
void CreateTypeList()
{
	for (int i=HuiFormatString.num;i<PartString.num;i++)
		HuiFormatString.add('t');
	for (int i=0;i<PartString.num;i++)
		if ((HuiFormatString[i] == 'c') || (HuiFormatString[i] == 'C'))
			HuiTypeList[i] = G_TYPE_BOOLEAN;
		else if (HuiFormatString[i] == 'i')
			HuiTypeList[i] = GDK_TYPE_PIXBUF;
		else
			HuiTypeList[i] = G_TYPE_STRING;
}

void configure_tree_view_columns(HuiControl *c, GtkWidget *view)
{
	for (int i=0;i<PartString.num;i++){
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;
		if (HuiFormatString[i] == 'C'){
   			renderer = gtk_cell_renderer_toggle_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(PartString[i]), renderer, "active", i, NULL);
			g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(i));
			g_signal_connect (G_OBJECT(renderer), "toggled", G_CALLBACK(list_toggle_callback), c);
		}else if (HuiFormatString[i] == 'c'){
   			renderer = gtk_cell_renderer_toggle_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(PartString[i]), renderer, "active", i, NULL);
		}else if (HuiFormatString[i] == 'i'){
   			renderer = gtk_cell_renderer_pixbuf_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(PartString[i]), renderer, "pixbuf", i, NULL);
		}else if (HuiFormatString[i] == 'T'){
			renderer = gtk_cell_renderer_text_new();
			g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(i));
			g_object_set(renderer, "editable", TRUE, NULL);
			g_signal_connect(renderer, "edited", G_CALLBACK(list_edited_callback), c);
			column = gtk_tree_view_column_new_with_attributes(sys_str(PartString[i]), renderer, "text", i, NULL);
		}else{
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes(sys_str(PartString[i]), renderer, "text", i, NULL);
		}
		gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	}
}

void OnGtkListActivate(GtkWidget *widget, void* a, void* b, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:activate");	}

void OnGtkListSelect(GtkTreeSelection *selection, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, (GtkWidget*)gtk_tree_selection_get_tree_view(selection), "hui:select", false);	}

void CHuiWindow::AddListView(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);

	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
	// "model"
	CreateTypeList();
	GtkListStore *store = gtk_list_store_newv(PartString.num, HuiTypeList);
	
	// "view"
	GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	g_signal_connect(G_OBJECT(view), "row-activated", G_CALLBACK(&OnGtkListActivate), this);

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(&OnGtkListSelect), this);

	if (OptionString.find("multiline") >= 0)
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	if (OptionString.find("nobar") >= 0)
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), false);

	// frame
	GtkWidget *frame = sw;
	if (border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), sw);
	}
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);
	
	HuiControl *c = _InsertControl_(view, x, y, width, height, id, HuiKindListView, frame);

	configure_tree_view_columns(c, view);
}

void CHuiWindow::AddTreeView(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);

	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	
	// "model"
	CreateTypeList();
	GtkTreeStore *store = gtk_tree_store_newv(PartString.num, HuiTypeList);
	
	// "view"
	GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	g_signal_connect(G_OBJECT(view),"row-activated",G_CALLBACK(&OnGtkListActivate),this);

	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(&OnGtkListSelect), this);
	
	if (OptionString.find("nobar") >= 0)
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), false);

	// frame
	GtkWidget *frame = sw;
	if (border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), sw);
	}
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);
	
	HuiControl *c = _InsertControl_(view, x, y, width, height, id, HuiKindTreeView, frame);
	
	configure_tree_view_columns(c, view);
}

void OnGtkIconListActivate(GtkWidget *widget, void* a, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:activate");	}

void CHuiWindow::AddIconView(const string &title,int x,int y,int width,int height,const string &id)
{
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
	_InsertControl_(view, x, y, width, height, id, HuiKindIconView, frame);
}

void CHuiWindow::AddProgressBar(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *pb = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pb), sys_str(PartString[0]));
	//g_signal_connect(G_OBJECT(pb), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
	_InsertControl_(pb, x, y, width, height, id, HuiKindProgressBar);
}


void OnGtkSliderChange(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget, "hui:change");	}

void CHuiWindow::AddSlider(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *s;
	if (height > width){
		s = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 1.0, 0.0001);
		gtk_range_set_inverted(GTK_RANGE(s), true);
	}else
		s = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.0001);
	gtk_scale_set_draw_value(GTK_SCALE(s), false);
	g_signal_connect(G_OBJECT(s), "value-changed", G_CALLBACK(&OnGtkSliderChange), this);
	_InsertControl_(s, x, y, width, height, id, HuiKindSlider);
}

void CHuiWindow::AddImage(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *im;
	if (PartString[0].num > 0){
		if (PartString[0][0] == '/')
			im = gtk_image_new_from_file(sys_str_f(PartString[0]));
		else
			im = gtk_image_new_from_file(sys_str_f(HuiAppDirectoryStatic + PartString[0]));
	}else
		im = gtk_image_new();
	_InsertControl_(im, x, y, width, height, id, HuiKindImage);
}



gboolean OnGtkAreaDraw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	NotifyWindowByWidget((CHuiWindow*)user_data, widget, "hui:redraw");
	return false;
}

/*void OnGtkAreaResize(GtkWidget *widget, GtkRequisition *requisition, gpointer user_data)
{	NotifyWindowByWidget((CHuiWindow*)user_data, widget, "hui:resize", false);	}*/

gboolean OnGtkAreaMouseMove(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	CHuiWindow *win = (CHuiWindow*)user_data;
	win->input.dx = (int)event->x - win->input.area_x;
	win->input.dy = (int)event->y - win->input.area_y;
	win->input.area_x = (int)event->x;
	win->input.area_y = (int)event->y;
	int mod = event->state;
	win->input.lb = ((mod & GDK_BUTTON1_MASK) > 0);
	win->input.mb = ((mod & GDK_BUTTON2_MASK) > 0);
	win->input.rb = ((mod & GDK_BUTTON3_MASK) > 0);
	NotifyWindowByWidget(win, widget, "hui:mouse-move", false);
	gdk_event_request_motions(event); // too prevent too many signals for slow message processing
	return false;
}

gboolean OnGtkAreaButtonDown(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	string msg = "hui:";
	if (event->button == 1)
		msg += "left";
	else if (event->button == 2)
		msg += "middle";
	else if (event->button == 3)
		msg += "right";
	if (event->type == GDK_2BUTTON_PRESS)
		msg += "-double-click";
	else
		msg += "-button-down";
	NotifyWindowByWidget((CHuiWindow*)user_data, widget, msg, false);
	return false;
}

gboolean OnGtkAreaButtonUp(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	string msg = "hui:";
	if (event->button == 1)
		msg += "left";
	else if (event->button == 2)
		msg += "middle";
	else if (event->button == 3)
		msg += "right";
	msg += "-button-up";
	NotifyWindowByWidget((CHuiWindow*)user_data, widget, msg, false);
	return false;
}

gboolean OnGtkAreaMouseWheel(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	CHuiWindow *win = (CHuiWindow*)user_data;
	if (win){
		if (event->direction == GDK_SCROLL_UP)
			win->input.dz = 1;
		else if (event->direction == GDK_SCROLL_DOWN)
			win->input.dz = -1;
		NotifyWindowByWidget(win, widget, "hui:mouse-wheel", false);
	}
	return false;
}

void CHuiWindow::AddDrawingArea(const string &title,int x,int y,int width,int height,const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *da = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(da), "draw", G_CALLBACK(OnGtkAreaDraw), this);
	//g_signal_connect(G_OBJECT(w), "key-press-event", G_CALLBACK(&key_press_event), this);
	//g_signal_connect(G_OBJECT(w), "key-release-event", G_CALLBACK(&key_release_event), this);
	//g_signal_connect(G_OBJECT(da), "size-request", G_CALLBACK(&OnGtkAreaResize), this);
	g_signal_connect(G_OBJECT(da), "motion-notify-event", G_CALLBACK(&OnGtkAreaMouseMove), this);
	g_signal_connect(G_OBJECT(da), "button-press-event", G_CALLBACK(&OnGtkAreaButtonDown), this);
	g_signal_connect(G_OBJECT(da), "button-release-event", G_CALLBACK(&OnGtkAreaButtonUp), this);
	g_signal_connect(G_OBJECT(da), "scroll-event", G_CALLBACK(&OnGtkAreaMouseWheel), this);
	//g_signal_connect(G_OBJECT(w), "focus-in-event", G_CALLBACK(&focus_in_event), this);
	int mask;
	g_object_get(G_OBJECT(da), "events", &mask, NULL);
	mask |= GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_VISIBILITY_NOTIFY_MASK | GDK_SCROLL_MASK; // GDK_POINTER_MOTION_HINT_MASK = "fewer motions"
	//mask = GDK_ALL_EVENTS_MASK;
	g_object_set(G_OBJECT(da), "events", mask, NULL);
	_InsertControl_(da, x, y, width, height, id, HuiKindDrawingArea);
	input_widget = da;
}


void CHuiWindow::AddControlTable(const string &title, int x, int y, int width, int height, const string &id)
{
	GetPartStrings(id, title);
	GtkWidget *t = gtk_table_new(height, width, false);
	gtk_table_set_row_spacings(GTK_TABLE(t), border_width);
	gtk_table_set_col_spacings(GTK_TABLE(t), border_width);
	_InsertControl_(t, x, y, width, height, id, HuiKindControlTable);
}

void CHuiWindow::EmbedDialog(const string &id, int x, int y)
{
#if 0
	CHuiWindow *dlg = HuiCreateResourceDialog(id, NULL, NULL);
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

void CHuiWindow::RemoveControl(const string &id)
{
	HuiControl *c = _GetControl_(id);
	/*for (int i=0;i<control.num;i++){
		GtkWidget *p = gtk_widget_get_parent(control[i]->widget);
		if (p == c->widget)
			RemoveControl(control[i]->ID);
	}*/
	for (int i=0;i<control.num;i++)
		if (control[i]->id == id){
			for (int j=i;j<control.num;j++)
				delete(control[j]);
			control.resize(i);
	}

	gtk_widget_destroy(c->widget);
}



//----------------------------------------------------------------------------------
// drawing

static int cur_font_size;
static string cur_font = "Sans";
static bool cur_font_bold, cur_font_italic;

void CHuiWindow::Redraw(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (c)
		gdk_window_invalidate_rect(gtk_widget_get_window(c->widget), NULL, false);
}

void CHuiWindow::RedrawRect(const string &_id, int x, int y, int w, int h)
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

static HuiDrawingContext hui_drawing_context; 

HuiDrawingContext *CHuiWindow::BeginDraw(const string &_id)
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

void HuiDrawingContext::End()
{
	if (!cr)
		return;
	
	cairo_destroy(cr);
}

void HuiDrawingContext::SetColor(const color &c)
{
	if (!cr)
		return;
	cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void HuiDrawingContext::SetLineWidth(float w)
{
	if (!cr)
		return;
	cairo_set_line_width(cr, w);
}

color HuiDrawingContext::GetThemeColor(int i)
{
	GtkStyle *style = gtk_widget_get_style(win->window);
	int x = (i / 5);
	int y = (i % 5);
	GdkColor c;
	if (x == 0)
		c = style->fg[y];
	else if (x == 1)
		c = style->bg[y];
	else if (x == 2)
		c = style->light[y];
	else if (x == 3)
		c = style->mid[y];
	else if (x == 4)
		c = style->dark[y];
	else if (x == 5)
		c = style->base[y];
	else if (x == 6)
		c = style->text[y];
	return color(1, (float)c.red / 65535.0f, (float)c.green / 65535.0f, (float)c.blue / 65535.0f);
}


void HuiDrawingContext::DrawPoint(float x, float y)
{
	if (!cr)
		return;
}

void HuiDrawingContext::DrawLine(float x1, float y1, float x2, float y2)
{
	if (!cr)
		return;
	cairo_move_to(cr, x1 + 0.5f, y1 + 0.5f);
	cairo_line_to(cr, x2 + 0.5f, y2 + 0.5f);
	cairo_stroke(cr);
}

void HuiDrawingContext::DrawLines(float *x, float *y, int num_lines)
{
	if (!cr)
		return;
	//cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	//cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, x[0], y[0]);
	for (int i=1;i<=num_lines;i++)  // <=  !!!
		cairo_line_to(cr, x[i], y[i]);
	cairo_stroke(cr);
}

void HuiDrawingContext::DrawLinesMA(Array<float> &x, Array<float> &y)
{
	DrawLines(&x[0], &y[0], x.num - 1);
}

void HuiDrawingContext::DrawPolygon(float *x, float *y, int num_points)
{
	if (!cr)
		return;
	cairo_move_to(cr, x[0], y[0]);
	for (int i=1;i<num_points;i++)
		cairo_line_to(cr, x[i], y[i]);
	cairo_close_path(cr);
	cairo_fill(cr);
}

void HuiDrawingContext::DrawPolygonMA(Array<float> &x, Array<float> &y)
{
	DrawPolygon(&x[0], &y[0], x.num);
}

void HuiDrawingContext::DrawStr(float x, float y, const string &str)
{
	if (!cr)
		return;
	cairo_move_to(cr, x, y);// + cur_font_size);
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, (char*)str.data, str.num);//.c_str(), -1);
	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + i2s(cur_font_size);
	PangoFontDescription *desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);
	
	//cairo_show_text(cr, str);
}

float HuiDrawingContext::GetStrWidth(const string &str)
{
	if (!cr)
		return 0;
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, (char*)str.data, str.num);//.c_str(), -1);
	string f = cur_font;
	if (cur_font_bold)
		f += " Bold";
	if (cur_font_italic)
		f += " Italic";
	f += " " + i2s(cur_font_size);
	PangoFontDescription *desc = pango_font_description_from_string(f.c_str());
	//PangoFontDescription *desc = pango_font_description_new();
	//pango_font_description_set_family(desc, "Sans");//cur_font);
	//pango_font_description_set_size(desc, 10);//cur_font_size);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	int w, h;
	pango_layout_get_size(layout, &w, &h);
	g_object_unref(layout);

	return (float)w / 1000.0f;
}

void HuiDrawingContext::DrawRect(float x, float y, float w, float h)
{
	if (!cr)
		return;
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
}

void HuiDrawingContext::DrawRect(const rect &r)
{
	if (!cr)
		return;
	cairo_rectangle(cr, r.x1, r.y1, r.width(), r.height());
	cairo_fill(cr);
}

void HuiDrawingContext::DrawCircle(float x, float y, float radius)
{
	if (!cr)
		return;
	cairo_arc(cr, x, y, radius, 0, 2 * pi);
	cairo_fill(cr);
}

void HuiDrawingContext::DrawImage(float x, float y, const Image &image)
{
#ifdef _X_USE_IMAGE_
	if (!cr)
		return;
	image.SetMode(Image::ModeBGRA);
	cairo_pattern_t *p = cairo_get_source(cr);
	cairo_pattern_reference(p);
	cairo_surface_t *img = cairo_image_surface_create_for_data((unsigned char*)image.data.data,
                                                         CAIRO_FORMAT_ARGB32,
                                                         image.width,
                                                         image.height,
	    image.width * 4);

	cairo_set_source_surface (cr, img, x, y);
	cairo_paint(cr);
	cairo_surface_destroy(img);
	cairo_set_source(cr, p);
	cairo_pattern_destroy(p);
#endif
}

void HuiDrawingContext::SetFont(const string &font, float size, bool bold, bool italic)
{
	if (!cr)
		return;
	//cairo_select_font_face(cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	//cairo_set_font_size(cr, size);
	if (font.num > 0)
		cur_font = font;
	if (size > 0)
		cur_font_size = size;
	cur_font_bold = bold;
	cur_font_italic = italic;
}

void HuiDrawingContext::SetFontSize(float size)
{
	if (!cr)
		return;
	//cairo_set_font_size(cr, size);
	cur_font_size = size;
}

void HuiDrawingContext::SetAntialiasing(bool enabled)
{
	if (!cr)
		return;
	if (enabled)
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_DEFAULT);
	else
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
}




//----------------------------------------------------------------------------------
// data exchanging functions for control items



// replace all the text
//    for all
void CHuiWindow::SetString(const string &_id, const string &str)
{
	if (id == _id)
		SetTitle(str);
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	//char *str2=sys_str(str);
	if (c->type==HuiKindText){
		GetPartStrings(_id, str);
		string s = sys_str(PartString[0]);
		if (OptionString.find("bold") >= 0)
			s = "<b>" + s + "</b>";
		else if (OptionString.find("italic") >= 0)
			s = "<i>" + s + "</i>";
		gtk_label_set_markup(GTK_LABEL(c->widget), s.c_str());
	}else if (c->type==HuiKindMultilineEdit){
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->widget));
		const char *str2 = sys_str(str);
		gtk_text_buffer_set_text(tb, str2, strlen(str2));
	}else if (c->type==HuiKindEdit){
		gtk_entry_set_text(GTK_ENTRY(c->widget),sys_str(str));
	}else if ((c->type==HuiKindListView)||(c->type==HuiKindTreeView)){
		AddString(_id, str);
	}else if (c->type==HuiKindComboBox){
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(c->widget),sys_str(str));
		c->_item_.add(dummy_iter);
	}else if (c->type==HuiKindProgressBar)
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(c->widget),sys_str(str));
	else if (c->type==HuiKindButton)
		gtk_button_set_label(GTK_BUTTON(c->widget),sys_str(str));
	else if (c->type==HuiKindSpinButton)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->widget), s2f(str));
	else if (c->type==HuiKindImage)
		SetImage(_id, str);
	allow_signal_level--;
}

// replace all the text with a numerical value (int)
//    for all
// select an item
//    for ComboBox, TabControl, ListView?
void CHuiWindow::SetInt(const string &_id, int n)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if ((c->type==HuiKindEdit) || (c->type==HuiKindText))
		SetString(_id, i2s(n));
	if (c->type==HuiKindTabControl){
		gtk_notebook_set_current_page(GTK_NOTEBOOK(c->widget),n);
		c->selected = n;
	}else if ((c->type==HuiKindListView) || (c->type==HuiKindTreeView)){
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(c->widget));
		if (n >= 0){
			gtk_tree_selection_select_iter(sel, &c->_item_[n]);
			GtkTreePath *path = gtk_tree_path_new_from_indices(n, -1);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(c->widget), path, NULL, false, 0, 0);
			gtk_tree_path_free(path);
		}else
			gtk_tree_selection_unselect_all(sel);
	}else if (c->type==HuiKindComboBox){
		gtk_combo_box_set_active(GTK_COMBO_BOX(c->widget), n);
	}else if (c->type==HuiKindSpinButton)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->widget), n);
	allow_signal_level--;
}

// replace all the text with a float
//    for all
void CHuiWindow::SetFloat(const string &_id, float f)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level ++;
	if (c->type == HuiKindProgressBar){
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(c->widget), clampf(f, 0, 1));
	}else if (c->type == HuiKindSlider)
		gtk_range_set_value(GTK_RANGE(c->widget), f);
	else if (c->type == HuiKindSpinButton)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->widget), f);
	else
		SetString(_id, f2s(f, num_float_decimals));
	allow_signal_level --;
}

void CHuiWindow::SetImage(const string &_id, const string &image)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (c->type==HuiKindButton){
		GtkWidget *im = (GtkWidget*)get_gtk_image(image, false);
		gtk_button_set_image(GTK_BUTTON(c->widget), im);
	}else if (c->type==HuiKindImage){
		gtk_image_set_from_file(GTK_IMAGE(c->widget), sys_str(image));
	}
	allow_signal_level--;
}

void set_list_cell(GtkListStore *store, GtkTreeIter &iter, int column, const string &str)
{
	GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), column);
	if (type == G_TYPE_STRING)
		gtk_list_store_set(store, &iter, column, sys_str(str), -1);
	else if (type == G_TYPE_BOOLEAN)
		gtk_list_store_set(store, &iter, column, (str == "1") || (str == "true"), -1);
	else if (type == GDK_TYPE_PIXBUF){
		GdkPixbuf *p = (GdkPixbuf*)get_gtk_image_pixbuf(str);
		if (p)
			gtk_list_store_set(store, &iter, column, p, -1);
	}
}

void set_tree_cell(GtkTreeStore *store, GtkTreeIter &iter, int column, const string &str)
{
	GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), column);
	if (type == G_TYPE_STRING)
		gtk_tree_store_set(store, &iter, column, sys_str(str), -1);
	else if (type == G_TYPE_BOOLEAN)
		gtk_tree_store_set(store, &iter, column, (str == "1") || (str == "true"), -1);
	else if (type == GDK_TYPE_PIXBUF){
		GdkPixbuf *p = (GdkPixbuf*)get_gtk_image_pixbuf(str);
		if (p)
			gtk_tree_store_set(store, &iter, column, p, -1);
	}
}

string tree_get_cell(GtkTreeModel *store, GtkTreeIter &iter, int column)
{
	string r;
	GType type = gtk_tree_model_get_column_type(store, column);
	if (type == G_TYPE_STRING){
		gchar *str;
		gtk_tree_model_get(store, &iter, column, &str, -1);
		r = str;
		g_free (str);
	}else if (type == G_TYPE_BOOLEAN){
		bool state;
		gtk_tree_model_get(store, &iter, column, &state, -1);
		r = b2s(state);
	}
	return r;
}

/*void set_list_row(HuiControl *c)
{
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
	gtk_list_store_append(store, &iter);
	for (int j=0;j<PartString.num;j++)
		set_list_cell(store, iter, j, PartString[j]);
}*/

// add a single line/string
//    for ComboBox, ListView, ListViewTree, ListViewIcons
void CHuiWindow::AddString(const string &_id, const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	//char *str2=sys_str(str);
	GtkTreeIter iter;
	if (c->type==HuiKindEdit)
		{}//gtk_entry_set_text(GTK_ENTRY(c->widget),sys_str(str));
	else if (c->type==HuiKindComboBox){
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(c->widget), NULL, sys_str(str));
		c->_item_.add(dummy_iter);
	}else if (c->type==HuiKindTreeView){
		GetPartStrings("", str);
		GtkTreeStore *store=GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		gtk_tree_store_append(store, &iter, NULL);
		for (int j=0;j<PartString.num;j++)
			set_tree_cell(store, iter, j, PartString[j]);
		c->_item_.add(iter);
	}else if (c->type==HuiKindListView){
		GetPartStrings("", str);
		GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		gtk_list_store_append(store, &iter);
		for (int j=0;j<PartString.num;j++)
			set_list_cell(store, iter, j, PartString[j]);
		c->_item_.add(iter);
	}else if (c->type==HuiKindIconView){
		GetPartStrings("", str);
		GtkTreeModel *model=gtk_icon_view_get_model(GTK_ICON_VIEW(c->widget));
		GtkWidget *im=gtk_image_new_from_file(sys_str(PartString[1]));
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0,sys_str(PartString[0]),1,gtk_image_get_pixbuf(GTK_IMAGE(im)),-1);
		c->_item_.add(iter);
	}
	allow_signal_level--;
}

// add a single line as a child in the tree of a ListViewTree
//    for ListViewTree
void CHuiWindow::AddChildString(const string &_id, int parent_row, const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (c->type==HuiKindTreeView){
		GtkTreeIter iter;
		GetPartStrings("", str);
		GtkTreeStore *store=GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		gtk_tree_store_append(store, &iter, &c->_item_[parent_row]);
		for (int j=0;j<PartString.num;j++)
			set_tree_cell(store, iter, j, PartString[j]);
		c->_item_.add(iter);
	}
	allow_signal_level--;
}

// change a single line in the tree of a ListViewTree
//    for ListViewTree
void CHuiWindow::ChangeString(const string &_id,int row,const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (c->type == HuiKindTreeView){
		GetPartStrings("", str);
		GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		/*for (int j=0;j<PartString.num;j++)
			gtk_tree_store_set(store, &c->_item_[row], j, sys_str(PartString[j]), -1);*/
		if (gtk_tree_store_iter_is_valid(store, &c->_item_[row]))
			for (int j=0;j<PartString.num;j++)
				set_tree_cell(store, c->_item_[row], j, PartString[j]);
	}else if (c->type == HuiKindListView){
		GetPartStrings("", str);
		GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		if (gtk_list_store_iter_is_valid(store, &c->_item_[row]))
			for (int j=0;j<PartString.num;j++)
				set_list_cell(store, c->_item_[row], j, PartString[j]);
	}
	allow_signal_level--;
}

// listview / treeview
string CHuiWindow::GetCell(const string &_id, int row, int column)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return "";
	string r;
	if (c->type==HuiKindTreeView){
		GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
		return tree_get_cell(store, c->_item_[row], column);
	}else if (c->type==HuiKindListView){
		GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
		return tree_get_cell(store, c->_item_[row], column);
	}
	return r;
}

// listview / treeview
void CHuiWindow::SetCell(const string &_id, int row, int column, const string &str)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (c->type==HuiKindTreeView){
		GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		if (gtk_tree_store_iter_is_valid(store, &c->_item_[row]))
			set_tree_cell(store, c->_item_[row], column, str);
	}else if (c->type==HuiKindListView){
		GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		if (gtk_list_store_iter_is_valid(store, &c->_item_[row]))
			set_list_cell(store, c->_item_[row], column, str);
	}
	allow_signal_level--;				
}

int col_f_to_i16(float f)
{	return (int)(f * 65535.0f);	}

float col_i16_to_f(int i)
{	return (float)i / 65535.0f;	}

void CHuiWindow::SetColor(const string &_id, const color &col)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (c->type == HuiKindColorButton){
		GdkColor gcol;
		gcol.red = col_f_to_i16(col.r);
		gcol.green = col_f_to_i16(col.g);
		gcol.blue = col_f_to_i16(col.b);
		gtk_color_button_set_color(GTK_COLOR_BUTTON(c->widget),&gcol);
		if (gtk_color_button_get_use_alpha(GTK_COLOR_BUTTON(c->widget)))
			gtk_color_button_set_alpha(GTK_COLOR_BUTTON(c->widget), col_f_to_i16(col.a));
	}
	allow_signal_level--;
}

// retrieve the text
//    for edit
string CHuiWindow::GetString(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return "";
	if (c->type==HuiKindText){}
	else if (c->type==HuiKindEdit)
		return de_sys_str(gtk_entry_get_text(GTK_ENTRY(c->widget)));
	else if (c->type==HuiKindMultilineEdit){
		GtkTextBuffer *tb=gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->widget));
		GtkTextIter is,ie;
		gtk_text_buffer_get_iter_at_offset(tb,&is,0);
		gtk_text_buffer_get_iter_at_offset(tb,&ie,-1);
		return de_sys_str(gtk_text_buffer_get_text(tb,&is,&ie,false));
	}else if (c->type==HuiKindSpinButton)
		return f2s(gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->widget)), gtk_spin_button_get_digits(GTK_SPIN_BUTTON(c->widget)));
	return "";
}

// retrieve the text as a numerical value (int)
//    for edit
// which item/line is selected?
//    for ComboBox, TabControl, ListView
int CHuiWindow::GetInt(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return -1;
	if ((c->type==HuiKindEdit) || (c->type==HuiKindText))
		return s2i(GetString(_id));
	if (c->type==HuiKindTabControl)
		return c->selected;//gtk_notebook_get_current_page(GTK_NOTEBOOK(c->widget));
	else if (c->type==HuiKindComboBox)
		return gtk_combo_box_get_active(GTK_COMBO_BOX(c->widget));
	else if ((c->type==HuiKindListView)||(c->type==HuiKindTreeView)){
		GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(c->widget));
		for (int j=0;j<c->_item_.num;j++)
			if (gtk_tree_selection_iter_is_selected(sel,&c->_item_[j]))
				return j;
		return -1;
	}else if (c->type==HuiKindIconView){
		GList *l=gtk_icon_view_get_selected_items(GTK_ICON_VIEW(c->widget));
		if (l){
			GtkTreePath *p=(GtkTreePath*)(l->data);
			g_list_free(l);
			int *indices=gtk_tree_path_get_indices(p),r;
			if (indices){
				r=indices[0];
				delete[](indices);
				return r;
			}else
				return -1;
		}else
			return -1;
	}else if (c->type==HuiKindSpinButton)
		return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(c->widget));
	return -1;
}

// retrieve the text as a numerical value (float)
//    for edit
float CHuiWindow::GetFloat(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return 0;
	if (c->type == HuiKindSlider)
		return gtk_range_get_value(GTK_RANGE(c->widget));
	else if (c->type == HuiKindSpinButton)
		return gtk_spin_button_get_value(GTK_SPIN_BUTTON(c->widget));
	return s2f(GetString(_id));
}

color CHuiWindow::GetColor(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return Black;
	color col = Black;
	if (c->type == HuiKindColorButton){
		GdkColor gcol;
		gtk_color_button_get_color(GTK_COLOR_BUTTON(c->widget), &gcol);
		col.r = col_i16_to_f(gcol.red);
		col.g = col_i16_to_f(gcol.green);
		col.b = col_i16_to_f(gcol.blue);
		if (gtk_color_button_get_use_alpha(GTK_COLOR_BUTTON(c->widget)))
			col.a = col_i16_to_f(gtk_color_button_get_alpha(GTK_COLOR_BUTTON(c->widget)));
	}
	return col;
}

// switch control to usable/unusable
//    for all
void CHuiWindow::Enable(const string &_id,bool enabled)
{
	_ToolbarEnable_(_id, enabled);
	if (menu)
		menu->EnableItem(_id, enabled);
	
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
    c->enabled = enabled;
	gtk_widget_set_sensitive(c->widget, enabled);
	allow_signal_level--;
}

// show/hide control
//    for all
void CHuiWindow::HideControl(const string &_id,bool hide)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (hide)
		gtk_widget_hide(c->widget);
	else
		gtk_widget_show(c->widget);
	allow_signal_level--;
}

// mark as "checked"
//    for CheckBox, ToolBarItemCheckable
void CHuiWindow::Check(const string &_id,bool checked)
{
	_ToolbarCheck_(_id, checked);
	HuiControl *c = _GetControl_(_id);
	if (menu)
		menu->CheckItem(_id, checked);
	if (!c)
		return;
	allow_signal_level++;
	if ((c->type == HuiKindCheckBox) || (c->type == HuiKindToggleButton) || (c->type == HuiKindRadioButton))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->widget), checked);
	allow_signal_level--;
}

// is marked as "checked"?
//    for CheckBox
bool CHuiWindow::IsChecked(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return _ToolbarIsChecked_(_id);
	if ((c->type == HuiKindCheckBox) || (c->type == HuiKindToggleButton) || (c->type == HuiKindRadioButton))
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->widget));
	return false;
}

// which lines are selected?
//    for ListView
Array<int> CHuiWindow::GetMultiSelection(const string &_id)
{
	Array<int> sel;
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return sel;
	if (c->type==HuiKindListView){
		GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(c->widget));
		gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
		for (int j=0;j<c->_item_.num;j++)
			if (gtk_tree_selection_iter_is_selected(s, &c->_item_[j])){
				sel.add(j);
			}
	}
	return sel;
}

void CHuiWindow::SetMultiSelection(const string &_id, Array<int> &sel)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	if (c->type==HuiKindListView){
		GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(c->widget));
		gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
		gtk_tree_selection_unselect_all(s);
		for (int j=0;j<sel.num;j++)
			gtk_tree_selection_select_iter(s, &c->_item_[sel[j]]);
	}
}

// delete all the content
//    for ComboBox, ListView
void CHuiWindow::Reset(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	allow_signal_level++;
	if (c->type==HuiKindTreeView){
		GtkTreeStore *store=GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		gtk_tree_store_clear(store);
	}else if (c->type==HuiKindListView){
		GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget)));
		gtk_list_store_clear(store);
	}else if (c->type==HuiKindIconView){
		//GtkTreeModel *model=gtk_icon_view_get_model(GTK_ICON_VIEW(c->widget));
		//gtk_tree_store_clear(store);
		msg_write("Todo:  CHuiWindow::ResetControl (IconView)  (Linux)");
	}else if (c->type==HuiKindComboBox){
		gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(c->widget));
	}
	c->_item_.clear();
	allow_signal_level--;
}

void CHuiWindow::CompletionAdd(const string &_id, const string &text)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	if (c->type == HuiKindEdit){
		GtkEntryCompletion *comp = gtk_entry_get_completion(GTK_ENTRY(c->widget));
		if (!comp){
			comp = gtk_entry_completion_new();
			//gtk_entry_completion_set_minimum_key_length(comp, 2);
			gtk_entry_completion_set_text_column(comp, 0);
			gtk_entry_set_completion(GTK_ENTRY(c->widget), comp);
		}
		GtkTreeModel *m = gtk_entry_completion_get_model(comp);
		if (!m){
			m = (GtkTreeModel*)gtk_list_store_new(1, G_TYPE_STRING);
			gtk_entry_completion_set_model(comp, m);
		}
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(m), &iter);
		set_list_cell(GTK_LIST_STORE(m), iter, 0, text);
	}
}

void CHuiWindow::CompletionClear(const string &_id)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	if (c->type == HuiKindEdit){
		gtk_entry_set_completion(GTK_ENTRY(c->widget), NULL);
	}
}

// expand a single row
//    for TreeView
void CHuiWindow::Expand(const string &_id, int row, bool expand)
{
	/*HuiControl *c = _GetControl_(_id);
	if (c->type == HuiKindTreeView){
		if (expand)
			gtk_tree_view_expand_row(GTK_TREE_VIEW(c->widget), c->_item_[row]);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(c->widget));
	}*/
	msg_todo("CHuiWindow::Expand()");
}

// expand all rows
//    for TreeView
void CHuiWindow::ExpandAll(const string &_id, bool expand)
{
	HuiControl *c = _GetControl_(_id);
	if (c->type == HuiKindTreeView){
		if (expand)
			gtk_tree_view_expand_all(GTK_TREE_VIEW(c->widget));
		else
			gtk_tree_view_collapse_all(GTK_TREE_VIEW(c->widget));
	}
}

// is column in tree expanded?
//    for TreeView
bool CHuiWindow::IsExpanded(const string &_id, int row)
{
	HuiControl *c = _GetControl_(_id);
	/*if (c->type == HuiKindTreeView)
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->widget));*/
	return false;
}

// for drawing area
#if 0
void CHuiWindow::SetInputHandler(const string &_id, input_handler_function *f)
{
	HuiControl *c = _GetControl_(_id);
	if (!c)
		return;
	input_widget = NULL;
	c->input_handler = f;
/*			GtkWidget *w = c->widget;
			g_signal_connect(G_OBJECT(w), "expose-event", G_CALLBACK(CallbackControl2), this);
			//g_signal_connect(G_OBJECT(w), "scroll-event", G_CALLBACK(&scroll_event), this);
			//g_signal_connect(G_OBJECT(w), "key-press-event", G_CALLBACK(&key_press_event), this);
			//g_signal_connect(G_OBJECT(w), "key-release-event", G_CALLBACK(&key_release_event), this);
			g_signal_connect(G_OBJECT(w), "size-request", G_CALLBACK(&size_request), this);
			g_signal_connect(G_OBJECT(w), "motion-notify-event", G_CALLBACK(&motion_notify_event), this);
			g_signal_connect(G_OBJECT(w), "button-press-event", G_CALLBACK(&button_press_event), this);
			g_signal_connect(G_OBJECT(w), "button-release-event", G_CALLBACK(&button_release_event), this);
			//g_signal_connect(G_OBJECT(w), "focus-in-event", G_CALLBACK(&focus_in_event), this);
			int mask;
			g_object_get(G_OBJECT(w), "events", &mask, NULL);
			mask |= GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_VISIBILITY_NOTIFY_MASK; // GDK_POINTER_MOTION_HINT_MASK = "fewer motions"
			//mask = GDK_ALL_EVENTS_MASK;
			g_object_set(G_OBJECT(w), "events", mask, NULL);*/
		//	input_widget = c->widget;
}
#endif

#endif
