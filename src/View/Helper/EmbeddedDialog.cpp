/*
 * EmbeddedDialog.cpp
 *
 *  Created on: 13.12.2012
 *      Author: michi
 */

#include "EmbeddedDialog.h"

EmbeddedDialog::EmbeddedDialog(HuiWindow *_win)
{
	win = _win;
}

EmbeddedDialog::~EmbeddedDialog()
{
}

void EmbeddedDialog::Enable(const string& id, bool enabled)
{
	win->Enable(id, enabled);
}

void EmbeddedDialog::Check(const string& id, bool checked)
{
	win->Check(id, checked);
}

bool EmbeddedDialog::IsChecked(const string& id)
{
	return win->IsChecked(id);
}

void EmbeddedDialog::Reset(const string& id)
{
	win->Reset(id);
}

void EmbeddedDialog::SetString(const string& id, const string& str)
{
	win->SetString(id, str);
}

void EmbeddedDialog::AddString(const string& id, const string& str)
{
	win->AddString(id, str);
}

void EmbeddedDialog::SetFloat(const string& id, float f)
{
	win->SetFloat(id, f);
}

void EmbeddedDialog::SetInt(const string& id, int i)
{
	win->SetInt(id, i);
}

string EmbeddedDialog::GetString(const string& id)
{
	return win->GetString(id);
}

string EmbeddedDialog::GetCell(const string& id, int row, int col)
{
	return win->GetCell(id, row, col);
}

float EmbeddedDialog::GetFloat(const string& id)
{
	return win->GetFloat(id);
}

int EmbeddedDialog::GetInt(const string& id)
{
	return win->GetInt(id);
}


