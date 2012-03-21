# module: hui

HUI_BIN  = temp/hui.a
HUI_DIR  = lib/hui/
HUI_OBJ  = temp/hui.o \
 temp/hui_language.o temp/hui_config.o temp/hui_resource.o temp/hui_utility.o temp/hui_input.o \
 temp/hui_common_dlg.o temp/hui_common_dlg_gtk.o temp/hui_common_dlg_win.o \
 temp/hui_menu.o temp/hui_menu_gtk.o temp/hui_menu_win.o \
 temp/hui_window.o temp/hui_window_gtk.o temp/hui_window_win.o \
 temp/hui_window_toolbar.o temp/hui_window_toolbar_gtk.o temp/hui_window_toolbar_win.o \
 temp/hui_window_control.o temp/hui_window_control_gtk.o temp/hui_window_control_win.o
HUI_CXXFLAGS =  `pkg-config --cflags gtk+-2.0` $(GLOBALFLAGS)


$(HUI_BIN) : $(HUI_OBJ)
	rm -f $@
	ar cq $@ $(HUI_OBJ)

temp/hui.o : $(HUI_DIR)hui.cpp
	$(CPP) -c $(HUI_DIR)hui.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_common_dlg.o : $(HUI_DIR)hui_common_dlg.cpp
	$(CPP) -c $(HUI_DIR)hui_common_dlg.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_common_dlg_gtk.o : $(HUI_DIR)hui_common_dlg_gtk.cpp
	$(CPP) -c $(HUI_DIR)hui_common_dlg_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_common_dlg_win.o : $(HUI_DIR)hui_common_dlg_win.cpp
	$(CPP) -c $(HUI_DIR)hui_common_dlg_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_language.o : $(HUI_DIR)hui_language.cpp
	$(CPP) -c $(HUI_DIR)hui_language.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_input.o : $(HUI_DIR)hui_input.cpp
	$(CPP) -c $(HUI_DIR)hui_input.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_resource.o : $(HUI_DIR)hui_resource.cpp
	$(CPP) -c $(HUI_DIR)hui_resource.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_utility.o : $(HUI_DIR)hui_utility.cpp
	$(CPP) -c $(HUI_DIR)hui_utility.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_config.o : $(HUI_DIR)hui_config.cpp
	$(CPP) -c $(HUI_DIR)hui_config.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_menu.o : $(HUI_DIR)hui_menu.cpp
	$(CPP) -c $(HUI_DIR)hui_menu.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_menu_gtk.o : $(HUI_DIR)hui_menu_gtk.cpp
	$(CPP) -c $(HUI_DIR)hui_menu_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_menu_win.o : $(HUI_DIR)hui_menu_win.cpp
	$(CPP) -c $(HUI_DIR)hui_menu_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window.o : $(HUI_DIR)hui_window.cpp
	$(CPP) -c $(HUI_DIR)hui_window.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_gtk.o : $(HUI_DIR)hui_window_gtk.cpp
	$(CPP) -c $(HUI_DIR)hui_window_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_win.o : $(HUI_DIR)hui_window_win.cpp
	$(CPP) -c $(HUI_DIR)hui_window_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_toolbar.o : $(HUI_DIR)hui_window_toolbar.cpp
	$(CPP) -c $(HUI_DIR)hui_window_toolbar.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_toolbar_gtk.o : $(HUI_DIR)hui_window_toolbar_gtk.cpp
	$(CPP) -c $(HUI_DIR)hui_window_toolbar_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_toolbar_win.o : $(HUI_DIR)hui_window_toolbar_win.cpp
	$(CPP) -c $(HUI_DIR)hui_window_toolbar_win.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_control.o : $(HUI_DIR)hui_window_control.cpp
	$(CPP) -c $(HUI_DIR)hui_window_control.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_control_gtk.o : $(HUI_DIR)hui_window_control_gtk.cpp
	$(CPP) -c $(HUI_DIR)hui_window_control_gtk.cpp -o $@ $(HUI_CXXFLAGS)

temp/hui_window_control_win.o : $(HUI_DIR)hui_window_control_win.cpp
	$(CPP) -c $(HUI_DIR)hui_window_control_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)hui.cpp : $(HUI_DIR)hui.h $(FILE_DIR)file.h
$(HUI_DIR)hui_menu.cpp : $(HUI_DIR)hui.h $(FILE_DIR)file.h
$(HUI_DIR)hui_window.cpp : $(HUI_DIR)hui.h $(FILE_DIR)file.h
$(HUI_DIR)hui.h : $(HUI_DIR)hui_config.h $(HUI_DIR)hui_menu.h $(HUI_DIR)hui_window.h
$(HUI_DIR)hui_window.h : $(HUI_DIR)hui_config.h

