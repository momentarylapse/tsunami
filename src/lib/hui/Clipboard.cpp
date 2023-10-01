/*
 * Clipboard.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */


#include "hui.h"

namespace hui {

namespace clipboard {

void copy(const string &buffer) {
#ifdef HUI_API_GTK
#if GTK_CHECK_VERSION(4,0,0)
	GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default()); //gtk_widget_get_clipboard(widget);
	gdk_clipboard_set_text(clipboard, buffer.c_str());
#else
	GtkClipboard *cb = gtk_clipboard_get_for_display(gdk_display_get_default(),GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(cb, (char*)buffer.data, buffer.num);
#endif
#endif
}

string paste() {
	string r;
#ifdef HUI_API_GTK
	//msg_write("--------a");
#if GTK_CHECK_VERSION(4,0,0)
	GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default()); //gtk_widget_get_clipboard(widget);

	GdkContentProvider *provider = gdk_clipboard_get_content(clipboard);

	GValue value = G_VALUE_INIT;
	g_value_init (&value, G_TYPE_STRING);

	// If the content provider does not contain text, we are not interested
	if (!gdk_content_provider_get_value (provider, &value, nullptr))
		return "";

	const char *buffer = g_value_get_string(&value);
	r = buffer;
	g_value_unset(&value);

#else
	GtkClipboard *cb = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	//msg_write("--------b");
	char *buffer = gtk_clipboard_wait_for_text(cb);
	//msg_write(*buffer);
	if (buffer) {
		r = buffer;
		g_free(buffer);
	}
#endif
	//msg_write(length);
#endif
	return r;
}

}

}
