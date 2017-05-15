/*
 * Clipboard.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */


#include "hui.h"

namespace hui
{

namespace Clipboard
{

void Copy(const string &buffer)
{
#ifdef HUI_API_WIN
	if (buffer.num < 1)
		return;
	if (!OpenClipboard(NULL))
		return;

	int nn=0; // Anzahl der Zeilenumbrueche
	for (int i=0;i<buffer.num;i++)
		if (buffer[i]=='\n')
			nn++;

	char *str=new char[buffer.num+nn+1];
	HGLOBAL hglbCopy;
	EmptyClipboard();

	// Pointer vorbereiten
	hglbCopy=GlobalAlloc(GMEM_MOVEABLE,sizeof(WCHAR)*(buffer.num+nn+1));
	if (!hglbCopy){
		CloseClipboard();
		return;
	}
	WCHAR *wstr=(WCHAR*)GlobalLock(hglbCopy);

	// befuellen
	int l=0;
	for (int i=0;i<buffer.num;i++){
		if (buffer[i]=='\n'){
			str[l]='\r';
			l++;
		}
		str[l]=buffer[i];
		l++;
	}
	str[l+1]=0;

	MultiByteToWideChar(CP_UTF8,0,(LPCSTR)str,-1,wstr,buffer.num+nn+1);
	delete(str);

	GlobalUnlock(hglbCopy);
	SetClipboardData(CF_UNICODETEXT,wstr);
	CloseClipboard();
#endif
#ifdef HUI_API_GTK
	GtkClipboard *cb=gtk_clipboard_get_for_display(gdk_display_get_default(),GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(cb, (char*)buffer.data, buffer.num);
#endif
}

string Paste()
{
	string r;
#ifdef HUI_API_WIN

	if (!OpenClipboard(NULL))
		return r;
	int nn=0;
	WCHAR *wstr=(WCHAR*)GetClipboardData(CF_UNICODETEXT);
	//char *str=(char*)GetClipboardData(CF_TEXT);
	CloseClipboard();

	int lll=WideCharToMultiByte(CP_UTF8,0,wstr,-1,NULL,0,NULL,NULL)+4;
		//HuiInfoBox(NULL,i2s(lll),"");
	char *str=new char[lll];
	WideCharToMultiByte(CP_UTF8,0,wstr,-1,(LPSTR)str,lll,NULL,NULL);
	delete[](wstr);

	r = str;

	// doppelte Zeilenumbrueche finden
	r.replace("\r", "");
#endif
#ifdef HUI_API_GTK
	//msg_write("--------a");
	GtkClipboard *cb = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	//msg_write("--------b");
	char *buffer = gtk_clipboard_wait_for_text(cb);
	//msg_write(*buffer);
	if (buffer){
		r = buffer;
		g_free(buffer);
	}
	//msg_write(length);
#endif
	return r;
}

}

}
