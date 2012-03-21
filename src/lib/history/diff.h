enum{
	DIFF_OP_EDIT,
	DIFF_OP_GROW,
	DIFF_OP_INSERT,
	DIFF_OP_DELETE,
	DIFF_OP_TRANSFER_SOURCE,
	DIFF_OP_TRANSFER_DEST,
};

struct HistoryObservable;

struct HistoryDiffOp
{
	void clear();
	void Apply(HistoryState *real_state, HistoryState *ref_state, bool down);
	
	void ApplyEdit(char *data, char *ref_data);
	void ApplyGrow(HistoryArray *array, DynamicArray *da, DynamicArray *ra, bool down);
	void ApplyInsert(HistoryArray *array, DynamicArray *da, DynamicArray *ra, bool down);
	void ApplyDelete(HistoryArray *array, DynamicArray *da, DynamicArray *ra, bool down);
	void ApplyTransfer(char *data_dest, char *data_src, bool down);
	
	int type;
	Array<int> addr;
	int size;
	char *buffer;
};

struct HistoryDiff
{
	HistoryDiff(){	memory_size = 0;	}
	void clear();
	void clear_shallow();
	void Test(HistoryState *real_state, HistoryState *ref_state);
	void TestHints(HistoryState *real_state, HistoryState *ref_state);
	void TestStruct(HistoryStruct *_struct, char *data, char *ref_data, const Array<int> &addr0);
	void TestArray(HistoryArray *array, char *data, char *ref_data, const Array<int> &addr0);
	void TestChunk(char *data, char *ref_data, int size, const Array<int> &addr0);
		
	void AddEdit(const char *data, int size, const Array<int> &addr, bool is_hint = false);
	void AddGrow(int dsize, const Array<int> &addr, bool is_hint = false);
	void AddInsert(int pos, const Array<int> &addr, bool is_hint = false);
	void AddDelete(int pos, const Array<int> &addr, bool is_hint = false);
	void AddTransfer(int size, const Array<int> &addr_dest, const Array<int> &addr_src, bool is_hint = false);
	
	void Apply(HistoryState *real_state, HistoryState *ref_state, bool down);

	void HintEdit(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int size);
	void HintGrow(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int dsize);
	void HintInsert(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int pos);
	void HintDelete(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr, int pos);
	void HintTransfer(HistoryState *real_state, HistoryState *ref_state, const Array<int> &addr_dest, const Array<int> &addr_src, int dsize);
	
	Array<HistoryDiffOp> op;
	Array<HistoryDiffOp> hint;
	int memory_size;
};
