# module: script

SCRIPT_BIN  = temp/script.a
SCRIPT_DIR  = lib/script/
SCRIPT_OBJ  = temp/script.o temp/pre_script.o temp/pre_script_lexical.o temp/pre_script_precompiler.o temp/pre_script_preprocessor.o temp/pre_script_parser.o temp/script_data.o temp/script_serializer.o temp/dasm.o
SCRIPT_CXXFLAGS =  `pkg-config --cflags gtk+-2.0` $(GLOBALFLAGS)
SCRIPT_DEP =  $(SCRIPT_DIR)script.h $(SCRIPT_DIR)dasm.h $(SCRIPT_DIR)pre_script.h $(SCRIPT_DIR)script_data.h

$(SCRIPT_BIN) : $(SCRIPT_OBJ) $(SCRIPT_DEP)
	rm -f $@
	ar cq $@ $(SCRIPT_OBJ)

temp/script.o : $(SCRIPT_DIR)script.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)script.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script.o : $(SCRIPT_DIR)pre_script.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)pre_script.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_lexical.o : $(SCRIPT_DIR)pre_script_lexical.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)pre_script_lexical.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_parser.o : $(SCRIPT_DIR)pre_script_parser.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)pre_script_parser.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_precompiler.o : $(SCRIPT_DIR)pre_script_precompiler.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)pre_script_precompiler.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/pre_script_preprocessor.o : $(SCRIPT_DIR)pre_script_preprocessor.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)pre_script_preprocessor.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_data.o : $(SCRIPT_DIR)script_data.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)script_data.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/script_serializer.o : $(SCRIPT_DIR)script_serializer.cpp $(SCRIPT_DEP)
	$(CPP) -c $(SCRIPT_DIR)script_serializer.cpp -o $@ $(SCRIPT_CXXFLAGS)

temp/dasm.o : $(SCRIPT_DIR)dasm.cpp
	$(CPP) -c $(SCRIPT_DIR)dasm.cpp -o $@ $(SCRIPT_CXXFLAGS)

#nix/nix.cpp : nix/nix.h
#nix/nix_types.cpp : nix/nix.h
#nix/nix_sound.cpp : nix/nix.h
#nix/nix_textures.cpp : nix/nix.h
#nix/nix_net.cpp : nix/nix.h
#nix/nix.h : nix/00_config.h nix/nix_config.h nix/nix_textures.h nix/nix_types.h nix/nix_sound.h nix/nix_net.h
#nix/nix_config.h : nix/00_config.h hui/hui.h file/file.h
#nix/nix_net.h : nix/nix.h

