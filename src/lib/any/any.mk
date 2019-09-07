# module: any

ANY_DIR = $(LIB_DIR)/any
ANY_MODULE  = $(ANY_DIR)/any.a
ANY_OBJ  = $(ANY_DIR)/any.o
ANY_CXXFLAGS = $(GLOBALFLAGS)


$(ANY_MODULE) : $(ANY_OBJ)
	rm -f $@
	ar cq $@ $(ANY_OBJ)

$(ANY_DIR)/any.o : $(ANY_DIR)/any.cpp
	$(CPP) -c $(ANY_DIR)/any.cpp -o $@ $(ANY_CXXFLAGS)

