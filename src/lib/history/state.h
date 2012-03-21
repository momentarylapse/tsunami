
struct HistoryObservable
{
	void clear();
	//void *GetChunk(const Array<int> &addr);
	//DynamicArray *GetArray(const Array<int> &addr, HistoryArray *&array);
	
	HistoryStruct *_struct;
	char *data;
};

struct HistoryState
{
	void clear();
	void Copy(HistoryState *state);
	char *GetChunk(const Array<int> &addr);
	DynamicArray *GetArray(const Array<int> &addr, HistoryArray *&array);
	//Array<int> GetArrayAddr(const void *p);
	
	Array<HistoryObservable> observable;
	int memory_size;
};
