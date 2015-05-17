/*
 * HuiControlTreeView.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControlTreeView.h"
#include "../hui.h"

#ifdef HUI_API_GTK

void list_toggle_callback(GtkCellRendererToggle *cell, gchar *path_string, gpointer data);
void list_edited_callback(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data);
extern GType HuiTypeList[64];
void CreateTypeList();
void configure_tree_view_columns(HuiControl *c, GtkWidget *view);
void OnGtkListActivate(GtkWidget *widget, void* a, void* b, gpointer data);
void OnGtkListSelect(GtkTreeSelection *selection, gpointer data);

void *get_gtk_image_pixbuf(const string &image); // -> hui_menu_gtk.cpp

HuiControlTreeView::HuiControlTreeView(const string &title, const string &id, HuiPanel *panel) :
	HuiControl(HUI_KIND_TREEVIEW, id)
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

string HuiControlTreeView::getString()
{
	return "";
}

void HuiControlTreeView::__setString(const string &str)
{
	__addString(str);
}

void set_tree_cell(GtkTreeStore *store, GtkTreeIter &_iter, int column, const string &str)
{
	GtkTreeIter iter = _iter;
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

string tree_get_cell(GtkTreeModel *store, GtkTreeIter &_iter, int column)
{
	GtkTreeIter iter = _iter;
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

void HuiControlTreeView::__addString(const string& str)
{
	GtkTreeIter iter;
	GetPartStrings("", str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_append(store, &iter, NULL);
	for (int j=0;j<PartString.num;j++)
		set_tree_cell(store, iter, j, PartString[j]);
	_item_.add(iter);
}

void HuiControlTreeView::__setInt(int i)
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (i >= 0){
		gtk_tree_selection_select_iter(sel, &_item_[i]);
		GtkTreePath *path = gtk_tree_path_new_from_indices(i, -1);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(widget), path, NULL, false, 0, 0);
		gtk_tree_path_free(path);
	}else
		gtk_tree_selection_unselect_all(sel);
}

int HuiControlTreeView::getInt()
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	for (int j=0;j<_item_.num;j++)
		if (gtk_tree_selection_iter_is_selected(sel, &_item_[j]))
			return j;
	return -1;
}

void HuiControlTreeView::__addChildString(int parent_row, const string& str)
{
	GtkTreeIter iter;
	GetPartStrings("", str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_append(store, &iter, &_item_[parent_row]);
	for (int j=0;j<PartString.num;j++)
		set_tree_cell(store, iter, j, PartString[j]);
	_item_.add(iter);
}

void HuiControlTreeView::__changeString(int row, const string& str)
{
	GetPartStrings("", str);
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	/*for (int j=0;j<PartString.num;j++)
		gtk_tree_store_set(store, &c->_item_[row], j, sys_str(PartString[j]), -1);*/
	if (gtk_tree_store_iter_is_valid(store, &_item_[row]))
		for (int j=0;j<PartString.num;j++)
			set_tree_cell(store, _item_[row], j, PartString[j]);
}

string HuiControlTreeView::getCell(int row, int column)
{
	if ((row < 0) or (row >= _item_.num))
		return "";
	GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	return tree_get_cell(store, _item_[row], column);
}

void HuiControlTreeView::__setCell(int row, int column, const string& str)
{
	if ((row < 0) or (row >= _item_.num))
		return;
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	if (gtk_tree_store_iter_is_valid(store, &_item_[row]))
		set_tree_cell(store, _item_[row], column, str);
}

Array<int> HuiControlTreeView::getMultiSelection()
{
	Array<int> sel;
	GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
	for (int j=0;j<_item_.num;j++)
		if (gtk_tree_selection_iter_is_selected(s, &_item_[j])){
			sel.add(j);
		}
	return sel;
}

void HuiControlTreeView::__setSelection(Array<int>& sel)
{
	GtkTreeSelection *s = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(s, GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_unselect_all(s);
	for (int j=0;j<sel.num;j++)
		gtk_tree_selection_select_iter(s, &_item_[sel[j]]);
}

void HuiControlTreeView::__reset()
{
	GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_tree_store_clear(store);
	_item_.clear();
}

void HuiControlTreeView::expand(int row, bool expand)
{
}

void HuiControlTreeView::expandAll(bool expand)
{
	if (expand)
		gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
	else
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(widget));
}

bool HuiControlTreeView::isExpanded(int row)
{
	/*HuiControl *c = _GetControl_(_id);
	if (c->type == HuiKindTreeView){
		if (expand)
			gtk_tree_view_expand_row(GTK_TREE_VIEW(c->widget), c->_item_[row]);
		else
			gtk_tree_view_collapse_row(GTK_TREE_VIEW(c->widget));
	}*/
	return false;
}

void HuiControlTreeView::__setOption(const string &op, const string &value)
{
	if (op == "nobar")
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), false);
	else if (op == "bar")
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), value._bool());
}

#endif
