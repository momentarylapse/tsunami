#include "../file/file.h"
#include "def.h"

static Array<HistoryStruct*> _struct_;
static HistoryStruct DummyStruct = {0, "dummy"};

HistoryStruct *HistoryGetStruct(const string &name)
{
	foreach(_struct_, s)
		if (s->name == name)
			return s;

	msg_error("HistoryGetStruct: unknown struct: " + name);
	return &DummyStruct;
}

HistoryStruct *HistoryCreateStruct(const string &name, int size)
{
	HistoryStruct *s = new HistoryStruct;
	_struct_.add(s);
	s->name = name;
	s->size = size;

	// update completely
	HistoryStructPart p;
	p.offset = 0;
	p.size = size;
	s->part.add(p);
	s->completely = true;
	
	return s;
}

void HistoryStruct::AddArray(int element_size, int offset)
{
	HistoryArray a;
	a.offset = offset;
	a.element_size = element_size;
	a.item_struct = NULL;
	a.auto_test = true;
	array.add(a);

	Ignore(offset, sizeof(DynamicArray));
}

void HistoryStruct::AddLazyArray(int element_size, int offset)
{
	AddArray(element_size, offset);
	array.back().auto_test = false;
}

void HistoryStruct::AddString(int offset)
{	AddArray(1, offset);	}

void HistoryStruct::AddStructArray(const string &sub_name, int offset)
{
	HistoryStruct *sub = HistoryGetStruct(sub_name);
	AddArray(sub->size, offset);
	array.back().item_struct = sub;
}

void HistoryStruct::Ignore(int _offset, int _size)
{
	//msg_write(format("ignore %d  %d", _offset, _size));
	if (_size <= 0)
		return;
	bool found = false;
	foreachi(part, p, i){
		//msg_write(format("   %d  %d", p.offset, p.size));
		// intervall inside p?
		if ((_offset >= p.offset) && (_offset + _size <= p.offset + p.size)){

			// new part after?
			HistoryStructPart *pp = &p;
			if (_offset + _size < p.offset + p.size){
				HistoryStructPart p2;
				p2.offset = _offset + _size;
				p2.size = p.offset + p.size - (_offset + _size);
				part.add(p2);
				pp = &part[i];
			}
			
			if (_offset > pp->offset){
				// shrink p to fit before
				pp->size = _offset - pp->offset;
			}else{
				part.erase(i);
			}
			completely = false;
			found = true;
		}
	}
	
	if (!found)
		msg_error("HistoryStruct::Ignore: un-ignorable intervall");
}

void HistoryStructReset(const string &name, void *data)
{
	msg_db_r("HistoryStructReset", 2);
	HistoryStruct *s = HistoryGetStruct(name);
	if (s){
		//msg_write("reset " + name);
		foreach(s->part, p)
			memset((char*)data + p.offset, 0, p.size);
	}
	msg_db_l(2);
}
