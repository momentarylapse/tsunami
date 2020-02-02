# module: nix

NIX_DIR = $(LIB_DIR)/nix
NIX_BIN  = $(NIX_DIR)/nix.a
NIX_OBJ  = $(NIX_DIR)/nix.o $(NIX_DIR)/nix_draw.o $(NIX_DIR)/nix_view.o $(NIX_DIR)/nix_textures.o \
$(NIX_DIR)/nix_light.o $(NIX_DIR)/nix_shader.o $(NIX_DIR)/nix_vertexbuffer.o
NIX_CXXFLAGS =  `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)

$(NIX_BIN) : $(NIX_OBJ)
	rm -f $@
	ar cq $@ $(NIX_OBJ)

$(NIX_DIR)/nix.o : $(NIX_DIR)/nix.cpp
	$(CPP) -c $(NIX_DIR)/nix.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix_draw.o : $(NIX_DIR)/nix_draw.cpp
	$(CPP) -c $(NIX_DIR)/nix_draw.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix_view.o : $(NIX_DIR)/nix_view.cpp
	$(CPP) -c $(NIX_DIR)/nix_view.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix_light.o : $(NIX_DIR)/nix_light.cpp
	$(CPP) -c $(NIX_DIR)/nix_light.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix_shader.o : $(NIX_DIR)/nix_shader.cpp
	$(CPP) -c $(NIX_DIR)/nix_shader.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix_vertexbuffer.o : $(NIX_DIR)/nix_vertexbuffer.cpp
	$(CPP) -c $(NIX_DIR)/nix_vertexbuffer.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix_textures.o : $(NIX_DIR)/nix_textures.cpp
	$(CPP) -c $(NIX_DIR)/nix_textures.cpp -o $@ $(NIX_CXXFLAGS)

$(NIX_DIR)/nix.cpp : $(NIX_DIR)/nix.h
$(NIX_DIR)/nix_textures.cpp : $(NIX_DIR)/nix.h
$(NIX_DIR)/nix_draw.cpp : $(NIX_DIR)/nix.h
$(NIX_DIR)/nix_view.cpp : $(NIX_DIR)/nix.h
$(NIX_DIR)/nix_net.cpp : $(NIX_DIR)/nix.h
$(NIX_DIR)/nix.h : $(LIB_DIR)/config.h $(NIX_DIR)/nix_config.h $(NIX_DIR)/nix_draw.h $(NIX_DIR)/nix_view.h $(NIX_DIR)/nix_textures.h
$(NIX_DIR)/nix_config.h : $(LIB_DIR)/config.h $(LIB_DIR)/hui/hui.h $(LIB_DIR)/file/file.h
#$(NIX_DIR)/nix_net.h : $(NIX_DIR)/nix.h

