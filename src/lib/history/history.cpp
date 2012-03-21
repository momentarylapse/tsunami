#include "../00_config.h"
#include "../file/file.h"
#include "history.h"

#define db_level(x)		//msg_write(x)
void db_mem(const string &x)
{
	//msg_write(x);
}

History::History()
{
	save_state = -2;
	current_state = -1;

	change_level = 0;
	change_aborted = false;
	
	change_callback = NULL;
	apply_callback = NULL;
}

History::~History()
{
	Clear();
}

void History::OnChange(history_callback *f)
{
	change_callback = f;
}

void History::OnApply(history_callback *f)
{
	apply_callback = f;
}

void History::AddData(const string &struct_name, void *data)
{
	HistoryObservable o;
	o._struct = HistoryGetStruct(struct_name);
	o.data = (char*)data;
	real_state.observable.add(o);
}

void History::Enable(bool _enabled)
{
	enabled = _enabled;
}

void History::Clear()
{
	msg_db_r("Hist.Clear", 1);

	// ref state
	ref_state.clear();

	// diffs
	foreach(diff, d)
		d.clear();
	diff.clear();
	
	save_state = -2;
	current_state = 0;

	msg_db_l(1);
}

void History::Reset(bool start)
{
	msg_db_r("Hist.Reset", 1);

	Clear();

	if (start){
		ref_state.Copy(&real_state);

		if (change_callback)
			change_callback();
	}

	enabled = start;
	save_state = 0;
	
	msg_db_l(1);
}

void History::ChangeBegin()
{
	if (change_level == 0)
		cur_diff.clear();
	
	change_level ++;
	db_level("+   " + i2s(change_level));
}

void History::ChangeAbort()
{
	change_aborted = true;
}

#ifdef _X_USE_HUI_

void HuiRunLater(int, history_callback*);
Array<History*> hist_later_stack;

static void later_end()
{
	if (hist_later_stack.num > 0){
		History *h = hist_later_stack.pop();
		h->ChangeEnd();
	}
}

void History::ChangeLater()
{
	hist_later_stack.add(this);
	ChangeBegin();
	Change();
	HuiRunLater(500, &later_end);
}
#endif

void History::Change()
{
	ChangeBegin();
	ChangeEnd();
}

void History::AddDiff(HistoryDiff *d)
{	
	// clear all diffs above
	for (int i=current_state;i<diff.num;i++)
		diff[i].clear();
	diff.resize(current_state);

	diff.add(*d);
	db_mem(format("mem: %d    (%d)", d->memory_size, d->op.num));
	d->clear_shallow();

	current_state = diff.num;
	if (change_callback)
		change_callback();
}

void History::HintEdit(const Array<int> &addr, int size)
{	if (enabled)	cur_diff.HintEdit(&real_state, &ref_state, addr, size);	}

void History::HintGrow(const Array<int> &addr, int dsize)
{	if (enabled)	cur_diff.HintGrow(&real_state, &ref_state, addr, dsize);	}

void History::HintInsert(const Array<int> &addr, int pos)
{	if (enabled)	cur_diff.HintInsert(&real_state, &ref_state, addr, pos);	}

void History::HintDelete(const Array<int> &addr, int pos)
{	if (enabled)	cur_diff.HintDelete(&real_state, &ref_state, addr, pos);	}

void History::HintTransfer(const Array<int> &addr_dest, const Array<int> &addr_src, int size)
{	if (enabled)	cur_diff.HintTransfer(&real_state, &ref_state, addr_dest, addr_src, size);	}

bool History::ChangeEnd()
{
	change_level --;
	db_level("-   " + i2s(change_level));

	bool has_changed = false;

	if (change_level == 0){
		if ((enabled) && (!change_aborted)){

			cur_diff.Test(&real_state, &ref_state);

			if (cur_diff.op.num > 0){
				has_changed = true;
				AddDiff(&cur_diff);
			}
		}
		change_aborted = false;
	}

	return has_changed;
}

void History::Undo()
{
	if (!IsUndoable())
		return;
	msg_db_r("Hist.Undo", 1);

	current_state --;
	diff[current_state].Apply(&real_state, &ref_state, true);
	
	if (apply_callback)
		apply_callback();
	
	msg_db_l(1);
}

void History::Redo()
{
	if (!IsRedoable())
		return;
	msg_db_r("Hist.Redo", 1);

	diff[current_state].Apply(&real_state, &ref_state, false);
	current_state ++;
	
	if (apply_callback)
		apply_callback();
	
	msg_db_l(1);
}

bool History::IsUndoable()
{
	return ((enabled) && (current_state > 0));
}

bool History::IsRedoable()
{
	if ((enabled) && (diff.num > 0))
		return current_state < diff.num;
	return false;
}

bool History::IsSaveState()
{
	return current_state == save_state;
}

void History::SetSaveState()
{
	save_state = current_state;
}

