/*
 * Effect.cpp
 *
 *  Created on: 10.12.2012
 *      Author: michi
 */

#include "Effect.h"

#include "Plugin.h"
#include "../Tsunami.h"

Effect::Effect()
{
	plugin = NULL;
}

void try_write_primitive_element(string &var_temp, sType *t, char *v)
{
	if (t == TypeInt)
		var_temp += i2s(*(int*)v);
	else if (t == TypeChar)
		var_temp += i2s(*(char*)v);
	else if (t == TypeFloat)
		var_temp += f2s(*(float*)v, 6);
	else if (t == TypeBool)
		var_temp += (*(bool*)v) ? "true" : "false";
	else if (t == TypeVector)
		var_temp += format("(%f %f %f)", *(float*)v, ((float*)v)[1], ((float*)v)[2]);
	else if (t == TypeComplex)
		var_temp += format("(%f %f)", *(float*)v, ((float*)v)[1]);
	else
		var_temp += "-------";
}

void try_write_element(EffectParam *p, sClassElement *e, char *v)
{
	p->name = e->Name;
	p->type = e->Type->Name;
	p->value = "";
	if (e->Type->IsArray){
		p->value += "[";
		for (int i=0;i<e->Type->ArrayLength;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->Type->SubType, &v[e->Offset + i * e->Type->SubType->Size]);
		}
		p->value += "]";
	}else if (e->Type->IsSuperArray){
		DynamicArray *a = (DynamicArray*)&v[e->Offset];
		p->value += format("[%d ", a->num);
		for (int i=0;i<a->num;i++){
			if (i > 0)
				p->value += " ";
			try_write_primitive_element(p->value, e->Type->SubType, &(((char*)a->data)[i * e->Type->SubType->Size]));
		}
		p->value += "]";
	}else
		try_write_primitive_element(p->value, e->Type, &v[e->Offset]);
}

string get_next(const string &var_temp, int &pos)
{
	int start = pos;
	for (int i=pos;i<var_temp.num;i++){
		if ((var_temp[i] != ' ') && (var_temp[i] != ']') && (var_temp[i] != ')') && (var_temp[i] != '[') && (var_temp[i] != '(')){
			start = i;
			break;
		}
	}
	for (int i=start;i<var_temp.num;i++){
		if ((var_temp[i] == ' ') || (var_temp[i] == ']') || (var_temp[i] == ')') || (var_temp[i] == '[') || (var_temp[i] == '(')){
			pos = i + 1;
			return var_temp.substr(start, i - start);
			break;
		}
	}
	return var_temp.substr(start, -1);
}

void try_read_primitive_element(const string &var_temp, int &pos, sType *t, char *v)
{
	if (t == TypeInt)
		*(int*)v = s2i(get_next(var_temp, pos));
	else if (t == TypeChar)
		*(char*)v = s2i(get_next(var_temp, pos));
	else if (t == TypeFloat)
		*(float*)v = s2f(get_next(var_temp, pos));
	else if (t == TypeComplex){
		((complex*)v)->x = s2f(get_next(var_temp, pos));
		((complex*)v)->y = s2f(get_next(var_temp, pos));
	}else if (t == TypeVector){
		((vector*)v)->x = s2f(get_next(var_temp, pos));
		((vector*)v)->y = s2f(get_next(var_temp, pos));
		((vector*)v)->z = s2f(get_next(var_temp, pos));
	}else if (t == TypeBool)
		*(bool*)v = (get_next(var_temp, pos) == "true");
}

void try_read_element(EffectParam &p, sClassElement *e, char *v)
{
	int pos = 0;
	if (e->Type->IsArray){
		for (int i=0;i<e->Type->ArrayLength;i++)
			try_read_primitive_element(p.value, pos, e->Type->SubType, &v[e->Offset + i * e->Type->SubType->Size]);
	}else if (e->Type->IsSuperArray){
		DynamicArray *a = (DynamicArray*)&v[e->Offset];
		int num = s2i(get_next(p.value, pos));
		a->resize(num);
		for (int i=0;i<num;i++)
			try_read_primitive_element(p.value, pos, e->Type->SubType, &(((char*)a->data)[i * e->Type->SubType->Size]));
	}else
		try_read_primitive_element(p.value, pos, e->Type, &v[e->Offset]);
}

void Effect::ExportData()
{
	msg_db_r("Effect.ExportData", 1);
	param.clear();
	if (plugin->data){
		sType *t = plugin->data_type;
		param.resize(t->Element.num);
		foreachi(sClassElement &e, t->Element, j)
			try_write_element(&param[j], &e, (char*)plugin->data);
	}
	msg_db_l(1);
}

void Effect::ImportData()
{
	msg_db_r("Effect.ImportData", 1);
	if (plugin->data){
		sType *t = plugin->data_type;
		foreach(sClassElement &e, t->Element)
			foreach(EffectParam &p, param)
				if ((e.Name == p.name) && (e.Type->Name == p.type))
					try_read_element(p, &e, (char*)plugin->data);
	}
	msg_db_l(1);
}

void Effect::ExportState()
{
	msg_db_r("Effect.ExportState", 1);
	if (plugin->state)
		memcpy(state, plugin->state, plugin->state_type->Size);
	msg_db_l(1);
}

void Effect::ImportState()
{
	msg_db_r("Effect.ImportState", 1);
	if (plugin->state)
		memcpy(plugin->state, state, plugin->state_type->Size);
	msg_db_l(1);
}

void Effect::Prepare()
{
	msg_db_r("Effect.Prepare", 1);
	LoadAndCompile();
	if (plugin){
		if (plugin->state){
			ImportData();
			state = new char[plugin->state_type->Size];
			// TODO (init)
			plugin->ResetState();
			ExportState();
		}
	}
	msg_db_l(1);
}

void Effect::CleanUp()
{
	msg_db_r("Effect.CleanUp", 1);
	if (plugin){
		if (plugin->state){
			ImportState();
			plugin->ResetState();
			// TODO (clear)
			ExportState();
			delete[]((char*)state);
		}
	}
	msg_db_l(1);
}



void Effect::LoadAndCompile()
{
	plugin = tsunami->plugins->GetPlugin(name);
}


void Effect::Apply(BufferBox &buf, Track *t)
{
	msg_db_r("Effect.Apply", 1);

	if (plugin){
		// run
		plugin->ResetData();
		ImportData();
		ImportState();
		if (plugin->type == Plugin::TYPE_EFFECT)
			plugin->f_process_track(&buf, t, 0);
		ExportState();
		t->root->UpdateSelection();
	}else{
		if (plugin->s)
			tsunami->log->Error(format(_("Beim Anwenden eines Effekt-Scripts (%s: %s %s)"), name.c_str(), plugin->s->ErrorMsgExt[0].c_str(), plugin->s->ErrorMsgExt[1].c_str()));
		else
			tsunami->log->Error(format(_("Beim Anwenden eines Effekt-Scripts (%s: Datei nicht ladbar)"), name.c_str()));
	}

	msg_db_l(1);
}
