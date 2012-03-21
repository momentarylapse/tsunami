#include "../file/file.h"
#include "history.h"

void db_mem(const string &);

// pointers to the array within struct buffer
int StateCopyArray(char *dest, char *source, HistoryArray *a)
{
	msg_db_r("StateCopyArray", 1);
	DynamicArray *da = (DynamicArray*)dest;
	DynamicArray *sa = (DynamicArray*)source;
	int mem = 0;

	// reset
	da->init(a->element_size);

	// really copy
	if (a->auto_test){
		da->assign(sa);
		mem = sa->num * sa->element_size;
	
		if (a->item_struct){
			//msg_write("s: " + a->item_struct->name);
			int e_offset = 0;
			for (int j=0;j<sa->num;j++){
		//		msg_write("e");
				char *e_dest   = ((char*)da->data + e_offset);
				char *e_source = ((char*)sa->data + e_offset);
				foreach(a->item_struct->array, sub_array)
					mem += StateCopyArray(e_dest + sub_array.offset,
					                      e_source + sub_array.offset, &sub_array);
				e_offset += a->element_size;
			}
		}
	}
	msg_db_l(1);
	return mem;
}

int StateCopyObservable(HistoryObservable *dest, HistoryObservable *src)
{
	msg_db_r("StateCopyObservable", 1);
	int mem = src->_struct->size;
	dest->data = new char[src->_struct->size];
	memcpy(dest->data, src->data, src->_struct->size);
	dest->_struct = src->_struct;

	// copy arrays
	foreach(src->_struct->array, a)
		mem += StateCopyArray(dest->data + a.offset, src->data + a.offset, &a);
	msg_db_l(1);
	return mem;
}

void StateClearStruct(char *data, HistoryStruct *s);

void StateClearArray(char *data, HistoryArray *a)
{
	msg_db_r("StateClearArray", 1);
	DynamicArray *da = (DynamicArray*)data;
	
	if (a->item_struct){
		//msg_write(a->item_struct->name);
		for (int i=0;i<da->num;i++)
			StateClearStruct((char*)da->data + i * a->element_size, a->item_struct);
	}

	da->clear();
	msg_db_l(1);
}

void StateClearStruct(char *data, HistoryStruct *s)
{
	msg_db_r("StateClearStruct", 1);
	foreach(s->array, a)
		StateClearArray(data + a.offset, &a);
	msg_db_l(1);
}

void HistoryObservable::clear()
{
	msg_db_r("Obs.clear", 1);

	StateClearStruct(data, _struct);
	delete[](data);
	
	msg_db_l(1);
}

DynamicArray *StateStructGetArray(HistoryStruct *_struct, char *data, const Array<int> &addr, int addr_level, HistoryArray *&rarray);

DynamicArray *StateArrayGetArray(HistoryArray *array, char *data, const Array<int> &addr, int addr_level, HistoryArray *&rarray)
{
	msg_db_r("StateGetArrayPointer", 2);
	DynamicArray *da = (DynamicArray*)data;
	DynamicArray *p = NULL;

	if (addr_level == addr.num){
		rarray = array;
		p = da;
	}

	// recursion
	if ((addr_level < addr.num) && (array->item_struct)){
		int n = addr[addr_level];
		p = StateStructGetArray(array->item_struct,
		                         (char*)da->data + n * da->element_size,
		                          addr, addr_level + 1, rarray);
	}

	msg_db_l(2);
	return p;
}

DynamicArray *StateStructGetArray(HistoryStruct *_struct, char *data, const Array<int> &addr, int addr_level, HistoryArray *&rarray)
{
	msg_db_r("StateStructGetArray", 2);
	DynamicArray *p = NULL;

	// sub-array
	if (addr_level < addr.num){
		int n = addr[addr_level];
		HistoryArray *a = &_struct->array[n];
		p = StateArrayGetArray(a, data + a->offset,
		                          addr, addr_level + 1, rarray);
	}
	
	msg_db_l(2);
	return p;
}

void HistoryState::clear()
{
	msg_db_r("State.clear", 1);

	foreach(observable, o)
		o.clear();
	observable.clear();
	
	memory_size = 0;
	
	msg_db_l(1);
}

DynamicArray *HistoryState::GetArray(const Array<int> &addr, HistoryArray *&array)
{
	msg_db_r("State.GetArray", 2);
	DynamicArray *p = NULL;

	int n = addr[0];
	if ((n >= 0) && (n < observable.num))
		p = StateStructGetArray(observable[n]._struct, observable[n].data, addr, 1, array);
	
	msg_db_l(2);
	return p;
}

char *HistoryState::GetChunk(const Array<int> &addr)
{
	msg_db_r("State.GetChunk", 2);
	char *p = NULL;

	if (addr.num == 2){
		// observable directly...
		int n = addr[0];
		if ((n >= 0) && (n < observable.num))
			p = observable[n].data + addr[1];
	}else if (addr.num > 2){
		// array...

		// find the deepest array
		int n = addr.num - 1;
		Array<int> a_addr = addr.sub(0, n - (n % 2));
		HistoryArray *array;
		DynamicArray *a = GetArray(a_addr, array);

		// find chunk
		if (a){
			if ((n % 2) == 0){
				// array directly
				p = (char*)a->data + addr[addr.num - 1];
			}else{
				// element in array
				p = (char*)a->data + a->element_size * addr[addr.num - 2] + addr[addr.num - 1];
			}
		}
	}
	
	msg_db_l(2);
	return p;
}

void HistoryState::Copy(HistoryState *state)
{
	msg_db_r("State.Copy", 1);

	clear();

	// create new state
	foreach(state->observable, o){
		HistoryObservable so;
		memory_size += StateCopyObservable(&so, &o);
		observable.add(so);
	}
	db_mem(format("mem: %d", memory_size));

	msg_db_l(1);
}
