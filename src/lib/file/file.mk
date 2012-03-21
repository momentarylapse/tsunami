# module: file

FILE_MODULE  = temp/file.a
FILE_DIR     = lib/file/
FILE_OBJ  = temp/file.o temp/msg.o temp/file_op.o temp/array.o temp/strings.o
FILE_CXXFLAGS = $(GLOBALFLAGS)


$(FILE_MODULE) : $(FILE_OBJ)
	rm -f $@
	ar cq $@ $(FILE_OBJ)

temp/file.o : $(FILE_DIR)file.cpp
	$(CPP) -c $(FILE_DIR)file.cpp -o $@ $(FILE_CXXFLAGS)

temp/msg.o : $(FILE_DIR)msg.cpp
	$(CPP) -c $(FILE_DIR)msg.cpp -o $@ $(FILE_CXXFLAGS)

temp/file_op.o : $(FILE_DIR)file_op.cpp
	$(CPP) -c $(FILE_DIR)file_op.cpp -o $@ $(FILE_CXXFLAGS)

temp/array.o : $(FILE_DIR)array.cpp
	$(CPP) -c $(FILE_DIR)array.cpp -o $@ $(FILE_CXXFLAGS)

temp/strings.o : $(FILE_DIR)strings.cpp
	$(CPP) -c $(FILE_DIR)strings.cpp -o $@ $(FILE_CXXFLAGS)
