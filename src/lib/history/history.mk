# module: history

HISTORY_BIN  = temp/history.a
HISTORY_DIR  = lib/history/
HISTORY_OBJ  = temp/history.o temp/history_def.o temp/history_state.o temp/history_diff.o
HISTORY_CXXFLAGS =  $(GLOBALFLAGS)


$(HISTORY_BIN) : $(HISTORY_OBJ)
	rm -f $@
	ar cq $@ $(HISTORY_OBJ)

temp/history.o : $(HISTORY_DIR)history.cpp $(HISTORY_DIR)history.h $(HISTORY_DIR)def.h  $(HISTORY_DIR)state.h $(HISTORY_DIR)diff.h
	$(CPP) -c $(HISTORY_DIR)history.cpp -o $@ $(HISTORY_CXXFLAGS)

temp/history_def.o : $(HISTORY_DIR)def.cpp $(HISTORY_DIR)def.h $(HISTORY_DIR)history.h $(HISTORY_DIR)diff.h
	$(CPP) -c $(HISTORY_DIR)def.cpp -o $@ $(HISTORY_CXXFLAGS)

temp/history_state.o : $(HISTORY_DIR)state.cpp $(HISTORY_DIR)state.h $(HISTORY_DIR)history.h $(HISTORY_DIR)def.h $(HISTORY_DIR)diff.h
	$(CPP) -c $(HISTORY_DIR)state.cpp -o $@ $(HISTORY_CXXFLAGS)

temp/history_diff.o : $(HISTORY_DIR)diff.cpp $(HISTORY_DIR)diff.h $(HISTORY_DIR)state.h $(HISTORY_DIR)history.h $(HISTORY_DIR)def.h
	$(CPP) -c $(HISTORY_DIR)diff.cpp -o $@ $(HISTORY_CXXFLAGS)
