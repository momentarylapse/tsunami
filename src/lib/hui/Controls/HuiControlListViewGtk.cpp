/*
 * HuiControlListView.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlListView.h"
#include "../hui.h"


#ifdef HUI_API_GTK

void *get_gtk_image_pixbuf(const string &image); // -> hui_menu_gtk.cpp
string tree_get_cell(GtkTreeModel *store, GtkTreeIter &iter, int column);

void list_toggle_callback(GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
	HuiControlListView *c = (HuiControlListView*)data;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
	gtk_tree_model_get_iter(model, &iter, path);
	bool state = (bool)gtk_cell_renderer_toggle_get_active(cell);
	state = !state;
	if (c->type == HUI_KIND_LISTVIEW)
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, state, -1);
	else if (c->type == HUI_KIND_TREEVIEW)
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, column, state, -1);

	c->panel->win->input.column = column;
	c->panel->win->input.row = s2i(path_string);
	c->notify("hui:change", false);
	gtk_tree_path_free(path);
}


void list_edited_callback(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data)
{
	HuiControlListView *c = (HuiControlListView*)data;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->widget));
	GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
	GtkTreeIter iter;
	gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
	gtk_tree_model_get_iter(model, &iter, path);
	if (c->type == HUI_KIND_LISTVIEW)
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, new_text, -1);
	else if (c->type == HUI_KIND_TREEVIEW)
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, column, new_text, -1);


	c->panel->win->input.column = column;
	c->panel->win->input.row = s2i(path_string);
	c->notify("hui:change", false);
	gtk_tree_path_free(path);
}

GType HuiTypeList[64];
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
{	((HuiControl*)data)->notify("hui:activate");	}

void OnGtkListSelect(GtkTreeSelection *selection, gpointer data)
{	((HuiControl*)data)->notify("hui:select", false);	}


void OnGtkListRowDeleted(GtkTreeModel *tree_model, GtkTreePath *path, gpointer user_data)
{
	HuiControlListView *lv = (HuiControlListView*)user_data;
	if (!lv->allow_change_messages)
		return;

	//msg_write("row del");
	gint *indices = gtk_tree_path_get_indices(path);

	if (indices[0] >= lv->row_target){
		lv->panel->win->input.row = indices[0] - 1;
		lv->panel->win->input.row_target = lv->row_target;
	}else{
		lv->panel->win->input.row = indices[0];
		lv->panel->win->input.row_target = lv->row_target - 1;
	}
	lv->notify("hui:move", false);
}

void OnGtkListRowInserted(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	HuiControlListView *lv = (HuiControlListView*)user_data;
	if (!lv->allow_change_messages)
		return;
	//msg_write("row insert");
	gint *indices = gtk_tree_path_get_indices(path);
	lv->row_target = indices[0];
}


HuiControlListView::HuiControlListView(const string &title, const string &id, HuiPanel *panel) :
	HuiControl(HUI_KIND_LISTVIEW, id)
{
	GetPartStrings(title);

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


	g_signal_connect(G_OBJECT(store), "row-inserted", G_CALLBACK(&OnGtkListRowInserted), this);
	g_signal_connect(G_OBJECT(store), "row-deleted", G_CALLBACK(&OnGtkListRowDeleted), this);
	allow_change_messages = true;

	// frame
	frame = sw;
	if (panel->border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), sw);
	}
	gtk_container_add(GTK_CONTAINER(sw), view);
	gtk_widget_show(sw);

	widget = view;

	configure_tree_view_columns(this, view);
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	setOptions(OptionString);
}

string HuiControlListView::getString()
{
	return "";
}

void HuiControlListView::__setString(const string &str)
{
	__addString(str);
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

void HuiControlListView::__addString(const string& str)
{
	allow_change_messages = false;
	GtkTreeIter iter;
	GetPartStrings(str);
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_append(store, &iter);
	for (int j=0;j<PartString.num;j++)
		set_list_cell(store, iter, j, PartString[j]);
	allow_change_messages = true;
}

void HuiControlListView::__setInt(int i)
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (i >= 0){
		GtkTreeIter iter;
		if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, i)){
			gtk_tree_selection_select_iter(sel, &iter);
			GtkTreePath *path = gtk_tree_path_new_from_indices(i, -1);
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget), path, NULL, false, 0, 0);
			gtk_tree_path_free(path);
		}
	}else
		gtk_tree_selection_unselect_all(sel);
}

int HuiControlListView::getInt()
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));

	for (int i=0; ; i++){
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, i))
			return -1;
		if (gtk_tree_selection_iter_is_selected(sel, &iter))
			return i;

	}
	return -1;
}

void HuiControlListView::__changeString(int row, const string& str)
{
	if (row < 0)
		return;
	GetPartStrings(str);
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, row))
		for (int j=0;j<PartString.num;j++)
			set_list_cell(store, iter, j, PartString[j]);
}

void HuiControlListView::__removeString(int row)
{
	if (row < 0)
		return;
	allow_change_messages = false;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, row))
		gtk_list_store_remove(store, &iter);
	allow_change_messages = true;
}

string HuiControlListView::getCell(int row, int column)
{
	if (row < 0)
		return "";
	GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	GtkTreeIter iter;
	if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, row))
		return "";
	return tree_get_cell(store, iter, column);
}

void HuiControlListView::__setCell(int row, int column, const string& str)
{
	if (row < 0)
		return;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;
	if (gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, row))
		set_list_cell(store, iter, column, str);
}

Array<int> HuiControlListView::getSelection()
{
	Array<int> selected;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));

	for (int i=0; ; i++){
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, i))
			return selected;
		if (gtk_tree_selection_iter_is_selected(sel, &iter))
			selected.add(i);

	}
	return selected;
}

void HuiControlListView::__setSelection(Array<int>& selected)
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_unselect_all(sel);
	for (int i: selected){
		GtkTreeIter iter;
		if (!gtk_tree_model_iter_nth_child(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)), &iter, NULL, i))
			continue;
		gtk_tree_selection_select_iter(sel, &iter);

	}
}

void HuiControlListView::__reset()
{
	allow_change_messages = false;
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_clear(store);
	allow_change_messages = true;
}

void HuiControlListView::__setOption(const string &op, const string &value)
{
	if ((op == "multiline") || (op == "select-multi")){
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	}else if (op == "select-single"){
		GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
	}else if (op == "nobar"){
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), false);
	}else if (op == "bar"){
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), value._bool());
	}else if (op == "reorderable"){
		gtk_tree_view_set_reorderable(GTK_TREE_VIEW(widget), true);
	}
}

#endif
