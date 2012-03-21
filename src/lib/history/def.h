#ifndef HISTORY_DEF_INCLUDED
#define HISTORY_DEF_INCLUDED

struct HistoryStruct;

struct HistoryArray
{
	int offset, element_size;
	HistoryStruct *item_struct;
	bool auto_test;
};

struct HistoryStructPart
{
	int offset, size;
};

struct HistoryStruct
{
	int size;
	string name;

	bool completely;
	Array<HistoryStructPart> part;

	Array<HistoryArray> array;

	void AddArray(int element_size, int offset);
	void AddLazyArray(int element_size, int offset);
	void AddString(int offset);
	void AddStructArray(const string &sub_name, int offset);
	void Ignore(int offset, int size);
};

HistoryStruct *HistoryCreateStruct(const string &name, int size);
HistoryStruct *HistoryGetStruct(const string &name);
void HistoryStructReset(const string &name, void *data);

#endif
