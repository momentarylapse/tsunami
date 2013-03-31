# module: script

SCRIPT_DIR = $(LIB_DIR)/script
SCRIPT_BIN  = $(SCRIPT_DIR)/script.a
SCRIPT_OBJ  = $(SCRIPT_DIR)/script.o \
 $(SCRIPT_DIR)/syntax/pre_script.o \
 $(SCRIPT_DIR)/syntax/pre_script_lexical.o \
 $(SCRIPT_DIR)/syntax/pre_script_precompiler.o \
 $(SCRIPT_DIR)/syntax/pre_script_preprocessor.o \
 $(SCRIPT_DIR)/syntax/pre_script_parser.o \
 $(SCRIPT_DIR)/lib/script_data.o \
 $(SCRIPT_DIR)/lib/script_data_file.o \
 $(SCRIPT_DIR)/lib/script_data_math.o \
 $(SCRIPT_DIR)/lib/script_data_threads.o \
 $(SCRIPT_DIR)/lib/script_data_nix.o \
 $(SCRIPT_DIR)/lib/script_data_hui.o \
 $(SCRIPT_DIR)/lib/script_data_net.o \
 $(SCRIPT_DIR)/lib/script_data_x.o \
 $(SCRIPT_DIR)/lib/script_data_image.o \
 $(SCRIPT_DIR)/lib/script_data_sound.o \
 $(SCRIPT_DIR)/compiler/script_serializer.o \
 $(SCRIPT_DIR)/compiler/script_compiler.o \
 $(SCRIPT_DIR)/asm/asm.o
SCRIPT_CXXFLAGS =  `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)
SCRIPT_DEP =  $(SCRIPT_DIR)/script.h $(SCRIPT_DIR)/asm/asm.h $(SCRIPT_DIR)/syntax/pre_script.h $(SCRIPT_DIR)/lib/script_data.h

$(SCRIPT_BIN) : $(SCRIPT_OBJ) $(SCRIPT_DEP)
	rm -f $@
	ar cq $@ $(SCRIPT_OBJ)

$(SCRIPT_DIR)/script.o : $(SCRIPT_DIR)/script.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/script.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/pre_script.o : $(SCRIPT_DIR)/syntax/pre_script.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/syntax/pre_script.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/syntax/pre_script_lexical.o : $(SCRIPT_DIR)/syntax/pre_script_lexical.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/syntax/pre_script_lexical.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/syntax/pre_script_parser.o : $(SCRIPT_DIR)/syntax/pre_script_parser.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/syntax/pre_script_parser.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/syntax/pre_script_precompiler.o : $(SCRIPT_DIR)/syntax/pre_script_precompiler.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/syntax/pre_script_precompiler.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/syntax/pre_script_preprocessor.o : $(SCRIPT_DIR)/syntax/pre_script_preprocessor.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/syntax/pre_script_preprocessor.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data.o : $(SCRIPT_DIR)/lib/script_data.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_math.o : $(SCRIPT_DIR)/lib/script_data_math.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_math.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_threads.o : $(SCRIPT_DIR)/lib/script_data_threads.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_threads.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_file.o : $(SCRIPT_DIR)/lib/script_data_file.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_file.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_hui.o : $(SCRIPT_DIR)/lib/script_data_hui.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_hui.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_nix.o : $(SCRIPT_DIR)/lib/script_data_nix.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_nix.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_net.o : $(SCRIPT_DIR)/lib/script_data_net.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_net.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_x.o : $(SCRIPT_DIR)/lib/script_data_x.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_x.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_image.o : $(SCRIPT_DIR)/lib/script_data_image.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_image.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/lib/script_data_sound.o : $(SCRIPT_DIR)/lib/script_data_sound.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/lib/script_data_sound.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/compiler/script_serializer.o : $(SCRIPT_DIR)/compiler/script_serializer.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/compiler/script_serializer.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/compiler/script_compiler.o : $(SCRIPT_DIR)/compiler/script_compiler.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)/compiler/script_compiler.cpp -o $@ $(SCRIPT_CXXFLAGS)

$(SCRIPT_DIR)/asm/asm.o : $(SCRIPT_DIR)/asm/asm.cpp
	$(CPP) -c $(SCRIPT_DIR)/asm/asm.cpp -o $@ $(SCRIPT_CXXFLAGS)

