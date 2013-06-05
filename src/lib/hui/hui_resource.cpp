#include "hui.h"
#include "hui_internal.h"

//----------------------------------------------------------------------------------
// resource functions

string str_unescape(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++){
		if ((str[i]=='\\')&&(str[i+1]=='n')){
			r += "\n";
			i ++;
		}else if ((str[i]=='\\')&&(str[i+1]=='\\')){
			r += "\\";
			i++;
		}else if ((str[i]=='\\')&&(str[i+1]=='?')){
			r += "?";
			i++;
		}else if ((str[i]=='\\')&&(str[i+1]=='t')){
			r += "\t";
			i++;
		}else if ((str[i]=='\\')&&(str[i+1]=='"')){
			r += "\"";
			i++;
		}else
			r.add(str[i]);
	}
	return r;
}


string str_escape(const string &str)
{
	string r;
	for (int i=0;i<str.num;i++){
		if (str[i] == '\t')
			r += "\\t";
		else if (str[i] == '\n')
			r += "\\n";
		else if (str[i] == '\\')
			r += "\\\\";
		else if (str[i] == '\"')
			r += "\\\"";
		else
			r.add(str[i]);
	}
	return r;
}

extern Array<HuiLanguage> _HuiLanguage_;
Array<HuiResource> _HuiResource_;

void LoadResourceCommand(CFile *f, HuiResourceCommand *c)
{
	c->type = f->ReadStr();
	c->id = f->ReadStr();
	c->image = f->ReadStr();
	c->enabled = f->ReadBool();
	c->i_param[0] = f->ReadInt();
	c->i_param[1] = f->ReadInt();
	c->i_param[2] = f->ReadInt();
	c->i_param[3] = f->ReadInt();
	c->i_param[4] = f->ReadInt();
	c->b_param[0] = f->ReadBool();
	c->b_param[1] = f->ReadBool();
	c->s_param[0] = f->ReadStr();
	c->s_param[1] = f->ReadStr();
}

void HuiLoadResource(const string &filename)
{
	msg_db_r("HuiLoadResource", 1);
	// dirty...
	_HuiResource_.clear();
	_HuiLanguage_.clear();

	CFile *f = OpenFile(filename);
	if (f){
		int ffv = f->ReadFileFormatVersion();
		int nres = f->ReadIntC();
		for (int i=0;i<nres;i++){
			HuiResource res;
			res.cmd.clear();
			f->ReadComment();
			LoadResourceCommand(f, &res);
			int n = f->ReadInt();
			for (int j=0;j<n;j++){
				HuiResourceCommand cmd;
				LoadResourceCommand(f, &cmd);
				res.cmd.add(cmd);
			}
			_HuiResource_.add(res);
		}

		// languages
		int nl = f->ReadIntC();
		for (int l=0;l<nl;l++){
			HuiLanguage hl;

			// Language
			hl.name = f->ReadStrC();

			//  NumIDs
			int n = f->ReadIntC();
			f->ReadComment(); // Text
			for (int i=0;i<n;i++){
				HuiLanguageCommand c;
				c.id = f->ReadStr();
				c.text = str_unescape(f->ReadStr());
				hl.cmd.add(c);
			}
			// Num Language Strings
			n = f->ReadIntC();
			// Text
			f->ReadComment();
			for (int i=0;i<n;i++){
				HuiLanguageTranslation s;
				s.orig = str_unescape(f->ReadStr());
				s.trans = str_unescape(f->ReadStr());
				hl.trans.add(s);
			}
			_HuiLanguage_.add(hl);
		}
		FileClose(f);
	}
}

HuiResource *HuiGetResource(const string &id)
{
	foreach(HuiResource &r, _HuiResource_)
		if (r.id == id)
			return &r;
	return NULL;
}

HuiWindow *HuiCreateResourceDialog(const string &id, HuiWindow *root)
{
	//return HuiCreateDialog("-dialog not found in resource-",200,100,root,true,mf);
	msg_db_f("HuiCreateResourceDialog",1);
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error(format("HuiCreateResourceDialog  (id=%s)  m(-_-)m",id.c_str()));
		return NULL;
	}
	
	msg_db_m("HuiResDialog",2);
	msg_db_m(i2s(res->i_param[0]).c_str(),2);
	msg_db_m(i2s(res->i_param[1]).c_str(),2);

	// dialog
	HuiWindow *dlg;
	if (res->type == "SizableDialog")
		dlg = HuiCreateSizableDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);
	else
		dlg = HuiCreateDialog(HuiGetLanguage(res->id), res->i_param[0], res->i_param[1], root, res->b_param[0]);

	// menu?
	if (res->s_param[0].num > 0)
		dlg->SetMenu(HuiCreateResourceMenu(res->s_param[0]));

	// toolbar?
	if (res->s_param[1].num > 0)
		dlg->ToolbarSetByID(res->s_param[1]);

	// controls
	foreach(HuiResourceCommand &cmd, res->cmd){
		//msg_db_m(format("%d:  %d / %d",j,(cmd->type & 1023),(cmd->type >> 10)).c_str(),4);
		if (res->type == "Dialog"){
			dlg->SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( dlg, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}else if (res->type == "SizableDialog"){
			//msg_write("insert " + cmd.id + " (" + cmd.type + ") into " + cmd.s_param[0]);
			dlg->SetTarget(cmd.s_param[0], cmd.i_param[4]);
			HuiWindowAddControl( dlg, cmd.type, HuiGetLanguage(cmd.id),
								cmd.i_param[0], cmd.i_param[1],
								cmd.i_param[2], cmd.i_param[3],
								cmd.id);
		}
		dlg->Enable(cmd.id, cmd.enabled);
		if (cmd.image.num > 0)
			dlg->SetImage(cmd.id, cmd.image);
	}
	msg_db_m("  \\(^_^)/",1);
	return dlg;
	
	/*msg_error(format("HuiCreateResourceDialog  (id=%d)  m(-_-)m",id));
	CHuiWindow *d=HuiCreateDialog(format("-dialog (id=%d) not found in resource-",id),300,200,root,true,mf);
	return d;*/
}

HuiMenu *_create_res_menu_(HuiResource *res, int &index, int num)
{
	msg_db_r("_create_res_menu_",2);
	HuiMenu *menu = new HuiMenu();
	//msg_db_out(2,i2s(n));
	for (int i=0;i<num;i++){
		//msg_db_out(2,i2s(j));
		HuiResourceCommand *cmd = &res->cmd[index];
		if (cmd->type == "Item")
			menu->AddItem(get_lang(cmd->id, "", true), cmd->id);
		if (cmd->type == "ItemImage")
			menu->AddItemImage(get_lang(cmd->id, "", true), cmd->image, cmd->id);
		if (cmd->type == "ItemCheckable")
			menu->AddItemCheckable(get_lang(cmd->id, "", true), cmd->id);
		if (cmd->type == "ItemSeparator")
			menu->AddSeparator();
		if (cmd->type == "ItemPopup"){
			index ++;
			HuiMenu *sub = _create_res_menu_(res, index, cmd->i_param[0]);
			menu->AddSubMenu(get_lang(cmd->id, "", true), cmd->id, sub);
			index --;
		}
		menu->EnableItem(cmd->id, cmd->enabled);
		index ++;
	}
	msg_db_l(2);
	return menu;
}

HuiMenu *HuiCreateResourceMenu(const string &id)
{
	msg_db_f("HuiCreateResourceMenu",1);
	msg_db_m(id.c_str(),2);
	
	HuiResource *res = HuiGetResource(id);
	if (!res){
		msg_error(format("HuiCreateResourceMenu  (id=%d)  m(-_-)m", id.c_str()).c_str());
		return NULL;
	}

	int i = 0;
	msg_db_m("  \\(^_^)/",1);
	HuiMenu *m = _create_res_menu_(res, i, res->i_param[0]);
	return m;
}

