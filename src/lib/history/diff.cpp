#include "../file/file.h"
#include "history.h"

#define db_out(x)		//msg_write(x)
#define db_mem(x)		//msg_write(x)
#define db_addr(x)		//msg_write(x)

void StateClearStruct(char *data, HistoryStruct *s);

bool mem_equal(const char *a, const char *b, int size, int &first, int &last)
{
	//msg_write(format("mem eq  %p  %p", a, b));
	// FIXME stupid
	for (int i=0;i<size;i++)
		if (a[i] != b[i]){
			first = i;
			last = i;
			//msg_write(format("ineq at %d", i));
			for (int j=size-1;j>i;j--)
				if (a[j] != b[j]){
					last = j;
					break;
				}
			return false;
		}
	return true;
}

void mem_set_xor(char *dest, const char *a, const char *b, int size)
{
	// FIXME stupid
	for (int i=0;i<size;i++)
		dest[i] = a[i] ^ b[i];
	//memcpy(chunk_data, ref_data, size);
}

string d2h_safe(const char *data, int size)
{
	if (size > 100)
		return d2h(data, 50) + "..." + d2h(data + size - 50, 50);
	return d2h(data, size);
}

void add_op(HistoryDiff *diff, int type, const char *data, int size, const Array<int> &addr, bool is_hint)
{
	HistoryDiffOp o;
	o.type = type;
	o.size = size;
	o.addr = addr;
	o.buffer = (char*)data;
	if (is_hint)
		diff->hint.add(o);
	else
		diff->op.add(o);
	diff->memory_size += sizeof(HistoryDiffOp) + addr.num * sizeof(int);
}

void HistoryDiff::AddEdit(const char *data, int size, const Array<int> &addr, bool is_hint)
{
	db_out(format("   diff edit  %d  ", size) + ia2s(addr) + "  " + d2h_safe(data, size));
	add_op(this, DIFF_OP_EDIT, data, size, addr, is_hint);
	memory_size += size;
}

void HistoryDiff::AddGrow(int dsize, const Array<int> &addr, bool is_hint)
{
	db_out(format("   diff grow  %d  ", dsize) + ia2s(addr));
	add_op(this, DIFF_OP_GROW, NULL, dsize, addr, is_hint);
}

void HistoryDiff::AddInsert(int pos, const Array<int> &addr, bool is_hint)
{
	db_out(format("   diff insert  %d  ", pos) + ia2s(addr));
	add_op(this, DIFF_OP_INSERT, NULL, pos, addr, is_hint);
}

void HistoryDiff::AddDelete(int pos, const Array<int> &addr, bool is_hint)
{
	db_out(format("   diff delete  %d  ", pos) + ia2s(addr));
	add_op(this, DIFF_OP_DELETE, NULL, pos, addr, is_hint);
}

void HistoryDiff::AddTransfer(int size, const Array<int> &addr_dest, const Array<int> &addr_src, bool is_hint)
{
	db_out(format("   diff transfer  %d  ", size) + ia2s(addr_dest) + "  <-  " + ia2s(addr_src));
	add_op(this, DIFF_OP_TRANSFER_SOURCE, NULL, size, addr_src,  is_hint);
	add_op(this, DIFF_OP_TRANSFER_DEST,   NULL, size, addr_dest, is_hint);
}

void HistoryDiff::TestChunk(char *data, char *ref_data, int size, const Array<int> &addr0)
{
	msg_db_r("TestChunk", 1);
	// TODO: if size > xxx then look for smaller changed parts...
	
	//db_addr(format("  test %d  ", size) + ia2s(addr0));
	int first, last;
	if (!mem_equal(data, ref_data, size, first, last)){
		size = last - first + 1;

		// save the OLD data as a chunk
		char *chunk_data = new char[size];
		Array<int> addr = addr0;
		addr.back() += first;
		mem_set_xor(chunk_data, data + first, ref_data + first, size);
		AddEdit(chunk_data, size, addr);

		// overwrite old ref with new data
		memcpy(ref_data + first, data + first, size);
	}
	msg_db_l(1);
}

void HistoryInitArrayElements(DynamicArray *da, int i0, int i1, HistoryStruct *item_struct)
{
	if (item_struct)
		for (int i=i0;i<i1;i++){
			foreach(item_struct->array, a){
				DynamicArray *sa = (DynamicArray*)((char*)da->data + i * da->element_size + a.offset);
				sa->init(a.element_size);
			}
		}
}

void HistoryDiff::TestArray(HistoryArray *array, char *data, char *ref_data, const Array<int> &addr0)
{
	msg_db_r("TestArray", 1);
	db_addr(ia2s(addr0));
	DynamicArray *da = (DynamicArray*)data;
	DynamicArray *ra = (DynamicArray*)ref_data;
	
	int size = da->num * da->element_size;

	if (da->num != ra->num){
		db_out("array num != num");
		int num0 = ra->num;

		if (ra->num > da->num){
			// shrunk -> save lost data
			int dsize = da->element_size * (ra->num - da->num);
			char *t = new char[dsize];
			memcpy(t, &((char*)ra->data)[size], dsize);
			AddEdit(t, dsize, addr0 + size);
		}

		AddGrow(da->num - ra->num, addr0);// array);

		ra->resize(da->num);

		// init subarrays (if grown)
		HistoryInitArrayElements(ra, num0, ra->num, array->item_struct);
			
	}


	if (array->item_struct){
		// struct[] but num=num
		for (int i=0;i<da->num;i++){
			TestStruct(array->item_struct,
			           (char*)da->data + i * da->element_size,
			           (char*)ra->data + i * da->element_size,
			           addr0 + i);
		}
	}else{
		// data[] with num=num
		if (size > 0)
			TestChunk((char*)da->data, (char*)ra->data, size, addr0 + 0);
	}

	msg_db_l(1);
}

void HistoryDiff::TestStruct(HistoryStruct *_struct, char *data, char *ref_data, const Array<int> &addr)
{
	msg_db_r("TestStruct", 1);
	db_out(_struct->name);
	db_addr(ia2s(addr));

	// test struct data itself
	foreach(_struct->part, p){
		db_addr(format("part %d %d", p->offset, p->size));
		TestChunk(data + p.offset,
		          ref_data + p.offset,
		          p.size, addr + p.offset);
	}

	// arrays
	foreachi(_struct->array, a, i)
		if (a.auto_test){
			TestArray(&a, data + a.offset,
			              ref_data + a.offset,
			              addr + i);
		}
	
	msg_db_l(1);
}

void HistoryDiff::TestHints(HistoryState *real_state, HistoryState *ref_state)
{
	msg_db_r("TestHints", 1);

	foreach(hint, h){
		if (h.type == DIFF_OP_EDIT){
			char *data = real_state->GetChunk(h.addr);
			TestChunk(data, h.buffer, h.size, h.addr);
			memory_size -= h.size;
		}else
			msg_error("HistoryDiff.TestHints: unhandled hint: " + i2s(h.type));
		h.clear();
		memory_size -= sizeof(HistoryDiffOp) - h.addr.num * sizeof(int);
	}
	hint.clear();
	
	msg_db_l(1);
}

void HistoryDiff::Test(HistoryState *real_state, HistoryState *ref_state)
{
	msg_db_r("Diff.Test", 1);

	// hints
	TestHints(real_state, ref_state);

	// auto testing
	foreachi(real_state->observable, o, i)
		TestStruct(o._struct, o.data, ref_state->observable[i].data, i);
	
	msg_db_l(1);
}

void HistoryDiffOp::ApplyEdit(char *data, char *ref_data)
{
	db_out(format("   apply edit  (%d)  ", size) + ia2s(addr));
	mem_set_xor(data, data, buffer, size);
	if (ref_data)
		mem_set_xor(ref_data, ref_data, buffer, size);
}

void HistoryDiffOp::ApplyGrow(HistoryArray *array, DynamicArray *da, DynamicArray *ra, bool down)
{
	int d_size = down ? (- size) : size;
	int old_size = da->num;
	int new_size = da->num + d_size;
	db_out(format("   apply grow  (%d -> %d)  ", old_size, new_size) + ia2s(addr));
	if (d_size < 0)
		db_out("lost: " + d2h_safe(((char*)da->data + new_size * da->element_size), - d_size * da->element_size));
	da->resize(new_size);
	if (ra)
		ra->resize(new_size);

	// initialize sub arrays
	if (d_size > 0){
		HistoryInitArrayElements(da, old_size, new_size, array->item_struct);
		if (ra)
			HistoryInitArrayElements(ra, old_size, new_size, array->item_struct);
	}
}

void HistoryDiffOp::ApplyInsert(HistoryArray *array, DynamicArray *da, DynamicArray *ra, bool down)
{
	if (down){
		int pos = size;
		db_out(format("   apply delete  (%d)  ", pos) + ia2s(addr));
		// let's hope, the array elements are empty!!!
		da->delete_single(pos);
		if (ra)
			ra->delete_single(pos);
	}else{
		int pos = size;
		db_out(format("   apply insert  (%d)  ", pos) + ia2s(addr));
		da->insert_blank(pos);
		HistoryInitArrayElements(da, pos, pos + 1, array->item_struct);
		if (ra){
			ra->insert_blank(pos);
			HistoryInitArrayElements(ra, pos, pos + 1, array->item_struct);
		}
	}
}

void HistoryDiffOp::ApplyDelete(HistoryArray *array, DynamicArray *da, DynamicArray *ra, bool down)
{
	ApplyInsert(array, da, ra, !down);
}

static HistoryDiffOp *last_transfer_part = NULL;

void HistoryDiffOp::ApplyTransfer(char *data_dest, char *data_src, bool down)
{
	db_out(format("   apply transfer  (%d)  ", size) + ia2s(addr) + "  <-  " + ia2s(last_transfer_part->addr));
	memcpy(data_dest, data_src, size);
	memset(data_src, 0, size);
}

void HistoryDiffOp::Apply(HistoryState *real_state, HistoryState *ref_state, bool down)
{
	msg_db_r("DiffOp.Apply", 1);

	if ((type == DIFF_OP_GROW) || (type == DIFF_OP_INSERT) || (type == DIFF_OP_DELETE)){
		// grow/shrink an array
		HistoryArray *array;
		DynamicArray *da = real_state->GetArray(addr, array);
		DynamicArray *ra = ref_state->GetArray(addr, array);
		if (!array->auto_test)
			ra = NULL;
		if (type == DIFF_OP_GROW){
			ApplyGrow(array, da, ra, down);
		}else if (type == DIFF_OP_INSERT){
			ApplyInsert(array, da, ra, down);
		}else if (type == DIFF_OP_DELETE){
			ApplyDelete(array, da, ra, down);
		}
	}else if (type == DIFF_OP_EDIT){
		// change data
		char *data = real_state->GetChunk(addr);
		char *ref_data = ref_state->GetChunk(addr);
		ApplyEdit(data, ref_data);
	}else if ((type == DIFF_OP_TRANSFER_SOURCE) || (type == DIFF_OP_TRANSFER_DEST)){
		if (last_transfer_part){
			// swap... (one way)
			char *data_dest = real_state->GetChunk(addr);
			char *data_src  = real_state->GetChunk(last_transfer_part->addr);
			ApplyTransfer(data_dest, data_src, down);
			last_transfer_part = NULL;
		}else
			last_transfer_part = this;
	}else
		msg_error("HistoryDiffOp.Apply: unhandled hint: " + i2s(type));
	
	msg_db_l(1);
}

void HistoryDiff::Apply(HistoryState *real_state, HistoryState *ref_state, bool down)
{
	msg_db_r("Diff.Apply", 1);

	if (down){
		foreachb(op, o)
			o.Apply(real_state, ref_state, down);
	}else{
		foreach(op, o)
			o.Apply(real_state, ref_state, down);
	}
	
	msg_db_l(1);
}

void HistoryDiffOp::clear()
{
	if (buffer)
		delete[](buffer);
}

void HistoryDiff::clear()
{
	msg_db_r("HistDiff.clear", 1);

	// operations
	foreach(op, o)
		o.clear();
	op.clear();

	// hints
	foreach(hint, h)
		h.clear();
	hint.clear();

	memory_size = 0;
	
	msg_db_l(1);
}

void HistoryDiff::clear_shallow()
{
	msg_db_r("HistDiff.clear_shallow", 1);
	op.clear();
	hint.clear();
	memory_size = 0;
	msg_db_l(1);
}








void HistoryDiff::HintEdit(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int size)
{
	TestHints(real_state, ref_state);
	char *data = real_state->GetChunk(addr);
	char *ref_data = new char[size];
	memcpy(ref_data, data, size);

	// test later!
	AddEdit(ref_data, size, addr, true);
}

void HistoryDiff::HintGrow(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int dsize)
{
	TestHints(real_state, ref_state);
	AddGrow(dsize, addr);
}

void HistoryDiff::HintInsert(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int pos)
{
	TestHints(real_state, ref_state);
	HistoryArray *array;
	DynamicArray *ra = ref_state->GetArray(addr, array);
	ra->insert_blank(pos);
	HistoryInitArrayElements(ra, pos, pos + 1, array->item_struct);
	AddInsert(pos, addr);
}

void HistoryDiff::HintDelete(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int pos)
{
	TestHints(real_state, ref_state);
	HistoryArray *array;
	DynamicArray *da = real_state->GetArray(addr, array);
	DynamicArray *ra = ref_state->GetArray(addr, array);
	if (array->item_struct){
		// FIXME ....might not be the best idea to kill the real data...
		char *data = (char*)da->data + pos * array->element_size;
		char *ref_data = (char*)ra->data + pos * array->element_size;
		// clear real state...
		StateClearStruct(data, array->item_struct);
		foreach(array->item_struct->part, p)
			memset(data + p.offset, 0, p.size);

		// find diffs
		TestStruct(array->item_struct, data, ref_data, addr + pos);
	}
	
	ra->delete_single(pos);
	AddDelete(pos, addr);
}

void HistoryDiff::HintTransfer(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr_dest, const Array<int> &addr_src, int size)
{
	TestHints(real_state, ref_state);
	AddTransfer(size, addr_dest, addr_src);
}
