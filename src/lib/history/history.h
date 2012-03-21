#include "def.h"
#include "state.h"
#include "diff.h"

typedef void history_callback();

class History
{
public:
	History();
	~History();

	void AddData(const string &struct_name, void *data);
	void OnChange(history_callback *f);
	void OnApply(history_callback *f);
	void Enable(bool enabled);

	void Reset(bool start = true);
	void SetSaveState();
	bool IsSaveState();
	bool IsUndoable();
	bool IsRedoable();
	void Undo();
	void Redo();

	void Change();
#ifdef _X_USE_HUI_
	void ChangeLater();
#endif
	void ChangeBegin();
	bool ChangeEnd();
	void ChangeAbort();

	void HintEdit(const Array<int> &addr, int size);
	void HintGrow(const Array<int> &addr, int dsize);
	void HintInsert(const Array<int> &addr, int pos);
	void HintTransfer(const Array<int> &addr_dest, const Array<int> &addr_src, int size);
	void HintDelete(const Array<int> &addr, int pos);

private:
	int change_level;
	bool change_aborted;
	bool enabled;

	HistoryState real_state;
	HistoryState ref_state;

	Array<HistoryDiff> diff;
	HistoryDiff cur_diff;
	int save_state, current_state;

	
	void AddDiff(HistoryDiff *d);

	void Clear();

	history_callback *apply_callback;
	history_callback *change_callback;
};
