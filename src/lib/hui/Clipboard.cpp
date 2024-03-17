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

static base::promise<string> clipboard_promise;

#if GTK_CHECK_VERSION(4,0,0)
void g_ready_callback(GObject* source_object, GAsyncResult* res, gpointer data) {
	auto p = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source_object), res, nullptr);
	if (p)
		clipboard_promise(p);
	else
		clipboard_promise.fail();
}
#endif

base::future<string> paste() {
	clipboard_promise.reset();
#ifdef HUI_API_GTK
#if GTK_CHECK_VERSION(4,0,0)
	GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default());
	// sadly, gdk_clipboard_get_content() + gdk_content_provider_get_value() seems to fail
	// need async :(
	gdk_clipboard_read_text_async(clipboard, nullptr, &g_ready_callback, nullptr);
#else
	GtkClipboard *cb = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	//msg_write("--------b");
	char *buffer = gtk_clipboard_wait_for_text(cb);
	//msg_write(*buffer);
	if (buffer) {
		clipboard_promise(buffer);
		g_free(buffer);
	}
#endif
#endif
	return clipboard_promise.get_future();
}

}

}
