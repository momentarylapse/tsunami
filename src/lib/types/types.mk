# module: types

TYPES_BIN  = temp/types.a
TYPES_DIR  = lib/types/
TYPES_OBJ  = temp/types.o temp/types_color.o temp/types_color.o temp/types_complex.o temp/types_vector.o \
   temp/types_matrix.o temp/types_matrix3.o temp/types_quaternion.o temp/types_plane.o temp/types_rect.o
TYPES_CXXFLAGS = $(GLOBALFLAGS)

$(TYPES_BIN) : $(TYPES_OBJ)
	rm -f $@
	ar cq $@ $(TYPES_OBJ)

temp/types.o : $(TYPES_DIR)types.cpp
	$(CPP) -c $(TYPES_DIR)types.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_color.o : $(TYPES_DIR)color.cpp
	$(CPP) -c $(TYPES_DIR)color.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_complex.o : $(TYPES_DIR)complex.cpp
	$(CPP) -c $(TYPES_DIR)complex.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_vector.o : $(TYPES_DIR)vector.cpp
	$(CPP) -c $(TYPES_DIR)vector.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_matrix.o : $(TYPES_DIR)matrix.cpp
	$(CPP) -c $(TYPES_DIR)matrix.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_matrix3.o : $(TYPES_DIR)matrix3.cpp
	$(CPP) -c $(TYPES_DIR)matrix3.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_quaternion.o : $(TYPES_DIR)quaternion.cpp
	$(CPP) -c $(TYPES_DIR)quaternion.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_plane.o : $(TYPES_DIR)plane.cpp
	$(CPP) -c $(TYPES_DIR)plane.cpp -o $@ $(TYPES_CXXFLAGS)

temp/types_rect.o : $(TYPES_DIR)rect.cpp
	$(CPP) -c $(TYPES_DIR)rect.cpp -o $@ $(TYPES_CXXFLAGS)
