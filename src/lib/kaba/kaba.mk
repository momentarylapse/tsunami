# module: kaba

KABA_DIR = $(LIB_DIR)/kaba
KABA_BIN  = $(KABA_DIR)/kaba.a
KABA_OBJ  = $(KABA_DIR)/kaba.o \
 $(KABA_DIR)/syntax/syntax_tree.o \
 $(KABA_DIR)/syntax/class.o \
 $(KABA_DIR)/syntax/lexical.o \
 $(KABA_DIR)/syntax/precompiler.o \
 $(KABA_DIR)/syntax/preprocessor.o \
 $(KABA_DIR)/syntax/parser.o \
 $(KABA_DIR)/syntax/implicit.o \
 $(KABA_DIR)/lib/exception.o \
 $(KABA_DIR)/lib/lib.o \
 $(KABA_DIR)/lib/lib_file.o \
 $(KABA_DIR)/lib/lib_math.o \
 $(KABA_DIR)/lib/lib_threads.o \
 $(KABA_DIR)/lib/lib_nix.o \
 $(KABA_DIR)/lib/lib_hui.o \
 $(KABA_DIR)/lib/lib_net.o \
 $(KABA_DIR)/lib/lib_x.o \
 $(KABA_DIR)/lib/lib_image.o \
 $(KABA_DIR)/lib/lib_sound.o \
 $(KABA_DIR)/compiler/serializer.o \
 $(KABA_DIR)/compiler/serializer_x86.o \
 $(KABA_DIR)/compiler/serializer_amd64.o \
 $(KABA_DIR)/compiler/serializer_arm.o \
 $(KABA_DIR)/compiler/compiler.o \
 $(KABA_DIR)/asm/asm.o
KABA_CXXFLAGS =  `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)
KABA_DEP =  $(KABA_DIR)/kaba.h \
 $(KABA_DIR)/asm/asm.h \
  $(KABA_DIR)/syntax/syntax_tree.h \
  $(KABA_DIR)/syntax/class.h \
  $(KABA_DIR)/syntax/lexical.h \
  $(KABA_DIR)/lib/lib.h

$(KABA_BIN) : $(KABA_OBJ) $(KABA_DEP)
	rm -f $@
	ar cq $@ $(KABA_OBJ)

$(KABA_DIR)/kaba.o : $(KABA_DIR)/kaba.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/kaba.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/syntax_tree.o : $(KABA_DIR)/syntax/syntax_tree.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/syntax_tree.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/class.o : $(KABA_DIR)/syntax/class.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/class.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/lexical.o : $(KABA_DIR)/syntax/lexical.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/lexical.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/parser.o : $(KABA_DIR)/syntax/parser.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/parser.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/implicit.o : $(KABA_DIR)/syntax/implicit.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/implicit.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/precompiler.o : $(KABA_DIR)/syntax/precompiler.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/precompiler.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/syntax/preprocessor.o : $(KABA_DIR)/syntax/preprocessor.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/syntax/preprocessor.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/exception.o : $(KABA_DIR)/lib/exception.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/exception.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib.o : $(KABA_DIR)/lib/lib.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_math.o : $(KABA_DIR)/lib/lib_math.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_math.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_threads.o : $(KABA_DIR)/lib/lib_threads.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_threads.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_file.o : $(KABA_DIR)/lib/lib_file.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_file.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_hui.o : $(KABA_DIR)/lib/lib_hui.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_hui.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_nix.o : $(KABA_DIR)/lib/lib_nix.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_nix.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_net.o : $(KABA_DIR)/lib/lib_net.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_net.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_x.o : $(KABA_DIR)/lib/lib_x.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_x.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_image.o : $(KABA_DIR)/lib/lib_image.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_image.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/lib/lib_sound.o : $(KABA_DIR)/lib/lib_sound.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/lib/lib_sound.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/compiler/serializer.o : $(KABA_DIR)/compiler/serializer.cpp $(KABA_DIR)/compiler/serializer.h $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/compiler/serializer.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/compiler/serializer_x86.o : $(KABA_DIR)/compiler/serializer_x86.cpp $(KABA_DIR)/compiler/serializer_x86.h $(KABA_DEP) $(KABA_DIR)/compiler/serializer.h $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/compiler/serializer_x86.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/compiler/serializer_amd64.o : $(KABA_DIR)/compiler/serializer_amd64.cpp $(KABA_DIR)/compiler/serializer_amd64.h $(KABA_DIR)/compiler/serializer_x86.h $(KABA_DEP) $(KABA_DIR)/compiler/serializer.h $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/compiler/serializer_amd64.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/compiler/serializer_arm.o : $(KABA_DIR)/compiler/serializer_arm.cpp $(KABA_DIR)/compiler/serializer_arm.h $(KABA_DEP) $(KABA_DIR)/compiler/serializer.h $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/compiler/serializer_arm.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/compiler/compiler.o : $(KABA_DIR)/compiler/compiler.cpp $(KABA_DEP)
	$(CPP) -c $(KABA_DIR)/compiler/compiler.cpp -o $@ $(KABA_CXXFLAGS)

$(KABA_DIR)/asm/asm.o : $(KABA_DIR)/asm/asm.cpp
	$(CPP) -c $(KABA_DIR)/asm/asm.cpp -o $@ $(KABA_CXXFLAGS)

