# module: hui

HUI_DIR = $(LIB_DIR)/hui
HUI_BIN  = $(HUI_DIR)/hui.a
HUI_OBJ  = $(HUI_DIR)/hui.o \
 $(HUI_DIR)/hui_language.o $(HUI_DIR)/hui_config.o $(HUI_DIR)/hui_resource.o $(HUI_DIR)/hui_utility.o \
 $(HUI_DIR)/hui_input.o $(HUI_DIR)/hui_error.o $(HUI_DIR)/hui_clipboard.o $(HUI_DIR)/HuiTimer.o \
 $(HUI_DIR)/hui_common_dlg.o $(HUI_DIR)/hui_common_dlg_gtk.o $(HUI_DIR)/hui_common_dlg_win.o \
 $(HUI_DIR)/HuiMenu.o $(HUI_DIR)/HuiMenuGtk.o $(HUI_DIR)/HuiMenuWin.o \
 $(HUI_DIR)/HuiWindow.o $(HUI_DIR)/HuiWindowGtk.o $(HUI_DIR)/HuiWindowWin.o \
 $(HUI_DIR)/HuiToolbar.o $(HUI_DIR)/HuiToolbarGtk.o $(HUI_DIR)/HuiToolbarWin.o \
 $(HUI_DIR)/hui_window_control.o $(HUI_DIR)/hui_window_control_gtk.o $(HUI_DIR)/hui_window_control_win.o \
 $(HUI_DIR)/HuiPainterGtk.o $(HUI_DIR)/Controls/HuiControl.o \
 $(HUI_DIR)/Controls/HuiControlCheckBoxGtk.o $(HUI_DIR)/Controls/HuiControlButtonGtk.o \
 $(HUI_DIR)/Controls/HuiControlColorButtonGtk.o $(HUI_DIR)/Controls/HuiControlEditGtk.o \
 $(HUI_DIR)/Controls/HuiControlComboBoxGtk.o $(HUI_DIR)/Controls/HuiControlGridGtk.o \
 $(HUI_DIR)/Controls/HuiControlDrawingAreaGtk.o $(HUI_DIR)/Controls/HuiControlGroupGtk.o \
 $(HUI_DIR)/Controls/HuiControlLabelGtk.o $(HUI_DIR)/Controls/HuiControlListViewGtk.o \
 $(HUI_DIR)/Controls/HuiControlMultilineEditGtk.o $(HUI_DIR)/Controls/HuiControlProgressBarGtk.o \
 $(HUI_DIR)/Controls/HuiControlRadioButtonGtk.o $(HUI_DIR)/Controls/HuiControlToggleButtonGtk.o \
 $(HUI_DIR)/Controls/HuiControlSpinButtonGtk.o $(HUI_DIR)/Controls/HuiControlSliderGtk.o \
 $(HUI_DIR)/Controls/HuiControlTabControlGtk.o $(HUI_DIR)/Controls/HuiControlTreeViewGtk.o \
 $(HUI_DIR)/Controls/HuiToolItemButtonGtk.o $(HUI_DIR)/Controls/HuiToolItemMenuButtonGtk.o \
 $(HUI_DIR)/Controls/HuiToolItemSeparatorGtk.o $(HUI_DIR)/Controls/HuiToolItemToggleButtonGtk.o \
 $(HUI_DIR)/Controls/HuiMenuItemGtk.o $(HUI_DIR)/Controls/HuiMenuItemSubmenuGtk.o \
 $(HUI_DIR)/Controls/HuiMenuItemSeparatorGtk.o $(HUI_DIR)/Controls/HuiMenuItemToggleGtk.o
HUI_CXXFLAGS =  `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)


$(HUI_BIN) : $(HUI_OBJ)
	rm -f $@
	ar cq $@ $(HUI_OBJ)

$(HUI_DIR)/hui.o : $(HUI_DIR)/hui.cpp
	$(CPP) -c $(HUI_DIR)/hui.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_main_gtk.o : $(HUI_DIR)/hui_main_gtk.cpp
	$(CPP) -c $(HUI_DIR)/hui_main_gtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_main_win.o : $(HUI_DIR)/hui_main_win.cpp
	$(CPP) -c $(HUI_DIR)/hui_main_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_common_dlg.o : $(HUI_DIR)/hui_common_dlg.cpp
	$(CPP) -c $(HUI_DIR)/hui_common_dlg.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_common_dlg_gtk.o : $(HUI_DIR)/hui_common_dlg_gtk.cpp
	$(CPP) -c $(HUI_DIR)/hui_common_dlg_gtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_common_dlg_win.o : $(HUI_DIR)/hui_common_dlg_win.cpp
	$(CPP) -c $(HUI_DIR)/hui_common_dlg_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_error.o : $(HUI_DIR)/hui_error.cpp
	$(CPP) -c $(HUI_DIR)/hui_error.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_clipboard.o : $(HUI_DIR)/hui_clipboard.cpp
	$(CPP) -c $(HUI_DIR)/hui_clipboard.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_language.o : $(HUI_DIR)/hui_language.cpp
	$(CPP) -c $(HUI_DIR)/hui_language.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_input.o : $(HUI_DIR)/hui_input.cpp
	$(CPP) -c $(HUI_DIR)/hui_input.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_resource.o : $(HUI_DIR)/hui_resource.cpp
	$(CPP) -c $(HUI_DIR)/hui_resource.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_utility.o : $(HUI_DIR)/hui_utility.cpp
	$(CPP) -c $(HUI_DIR)/hui_utility.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_config.o : $(HUI_DIR)/hui_config.cpp
	$(CPP) -c $(HUI_DIR)/hui_config.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiMenu.o : $(HUI_DIR)/HuiMenu.cpp
	$(CPP) -c $(HUI_DIR)/HuiMenu.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiMenuGtk.o : $(HUI_DIR)/HuiMenuGtk.cpp
	$(CPP) -c $(HUI_DIR)/HuiMenuGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiMenuWin.o : $(HUI_DIR)/HuiMenuWin.cpp
	$(CPP) -c $(HUI_DIR)/HuiMenuWin.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiWindow.o : $(HUI_DIR)/HuiWindow.cpp
	$(CPP) -c $(HUI_DIR)/HuiWindow.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiWindowGtk.o : $(HUI_DIR)/HuiWindowGtk.cpp
	$(CPP) -c $(HUI_DIR)/HuiWindowGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiWindowWin.o : $(HUI_DIR)/HuiWindowWin.cpp
	$(CPP) -c $(HUI_DIR)/HuiWindowWin.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiToolbar.o : $(HUI_DIR)/HuiToolbar.cpp
	$(CPP) -c $(HUI_DIR)/HuiToolbar.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiToolbarGtk.o : $(HUI_DIR)/HuiToolbarGtk.cpp
	$(CPP) -c $(HUI_DIR)/HuiToolbarGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiToolbarWin.o : $(HUI_DIR)/HuiToolbarWin.cpp
	$(CPP) -c $(HUI_DIR)/HuiToolbarWin.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_window_control.o : $(HUI_DIR)/hui_window_control.cpp
	$(CPP) -c $(HUI_DIR)/hui_window_control.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_window_control_gtk.o : $(HUI_DIR)/hui_window_control_gtk.cpp
	$(CPP) -c $(HUI_DIR)/hui_window_control_gtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/hui_window_control_win.o : $(HUI_DIR)/hui_window_control_win.cpp
	$(CPP) -c $(HUI_DIR)/hui_window_control_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControl.o : $(HUI_DIR)/Controls/HuiControl.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControl.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlButtonGtk.o : $(HUI_DIR)/Controls/HuiControlButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlCheckBoxGtk.o : $(HUI_DIR)/Controls/HuiControlCheckBoxGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlCheckBoxGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlComboBoxGtk.o : $(HUI_DIR)/Controls/HuiControlComboBoxGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlComboBoxGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlDrawingAreaGtk.o : $(HUI_DIR)/Controls/HuiControlDrawingAreaGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlDrawingAreaGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlEditGtk.o : $(HUI_DIR)/Controls/HuiControlEditGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlEditGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlGridGtk.o : $(HUI_DIR)/Controls/HuiControlGridGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlGridGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlGroupGtk.o : $(HUI_DIR)/Controls/HuiControlGroupGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlGroupGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlLabelGtk.o : $(HUI_DIR)/Controls/HuiControlLabelGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlLabelGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlListViewGtk.o : $(HUI_DIR)/Controls/HuiControlListViewGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlListViewGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlMultilineEditGtk.o : $(HUI_DIR)/Controls/HuiControlMultilineEditGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlMultilineEditGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlProgressBarGtk.o : $(HUI_DIR)/Controls/HuiControlProgressBarGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlProgressBarGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlRadioButtonGtk.o : $(HUI_DIR)/Controls/HuiControlRadioButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlRadioButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlSliderGtk.o : $(HUI_DIR)/Controls/HuiControlSliderGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlSliderGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlSpinButtonGtk.o : $(HUI_DIR)/Controls/HuiControlSpinButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlSpinButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlTabControlGtk.o : $(HUI_DIR)/Controls/HuiControlTabControlGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlTabControlGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlColorButtonGtk.o : $(HUI_DIR)/Controls/HuiControlColorButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlColorButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlToggleButtonGtk.o : $(HUI_DIR)/Controls/HuiControlToggleButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlToggleButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlTreeViewGtk.o : $(HUI_DIR)/Controls/HuiControlTreeViewGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlTreeViewGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlToolItemButtonGtk.o : $(HUI_DIR)/Controls/HuiControlToolItemButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlToolItemButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlToolItemMenuButtonGtk.o : $(HUI_DIR)/Controls/HuiControlToolItemMenuButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlToolItemMenuButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlToolItemSeparatorGtk.o : $(HUI_DIR)/Controls/HuiControlToolItemSeparatorGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlToolItemSeparatorGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlToolItemToggleButtonGtk.o : $(HUI_DIR)/Controls/HuiControlToolItemToggleButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlToolItemToggleButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlMenuItemGtk.o : $(HUI_DIR)/Controls/HuiControlMenuItemGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlMenuItemGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlMenuItemSubmenuGtk.o : $(HUI_DIR)/Controls/HuiControlMenuItemSubmenuGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlMenuItemSubmenuGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/HuiControlMenuItemToggleGtk.o : $(HUI_DIR)/Controls/HuiControlMenuItemToggleGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/HuiControlMenuItemToggleGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiPainterGtk.o : $(HUI_DIR)/HuiPainterGtk.cpp
	$(CPP) -c $(HUI_DIR)/HuiPainterGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/HuiTimer.o : $(HUI_DIR)/HuiTimer.cpp
	$(CPP) -c $(HUI_DIR)/HuiTimer.cpp -o $@ $(HUI_CXXFLAGS)


