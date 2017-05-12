# module: hui

HUI_DIR = $(LIB_DIR)/hui
HUI_BIN  = $(HUI_DIR)/hui.a
HUI_OBJ  = $(HUI_DIR)/hui.o \
 $(HUI_DIR)/language.o $(HUI_DIR)/Config.o $(HUI_DIR)/resource.o $(HUI_DIR)/utility.o \
 $(HUI_DIR)/input.o $(HUI_DIR)/error.o $(HUI_DIR)/clipboard.o $(HUI_DIR)/Timer.o \
 $(HUI_DIR)/common_dlg.o $(HUI_DIR)/common_dlg_gtk.o $(HUI_DIR)/common_dlg_win.o \
 $(HUI_DIR)/Menu.o $(HUI_DIR)/MenuGtk.o $(HUI_DIR)/MenuWin.o \
 $(HUI_DIR)/Window.o $(HUI_DIR)/WindowGtk.o $(HUI_DIR)/WindowWin.o \
 $(HUI_DIR)/Toolbar.o $(HUI_DIR)/ToolbarGtk.o $(HUI_DIR)/ToolbarWin.o \
 $(HUI_DIR)/window_control.o $(HUI_DIR)/window_control_gtk.o $(HUI_DIR)/window_control_win.o \
 $(HUI_DIR)/Panel.o $(HUI_DIR)/PainterGtk.o $(HUI_DIR)/Controls/Control.o \
 $(HUI_DIR)/Controls/ControlCheckBoxGtk.o $(HUI_DIR)/Controls/ControlButtonGtk.o \
 $(HUI_DIR)/Controls/ControlColorButtonGtk.o $(HUI_DIR)/Controls/ControlEditGtk.o \
 $(HUI_DIR)/Controls/ControlComboBoxGtk.o $(HUI_DIR)/Controls/ControlGridGtk.o \
 $(HUI_DIR)/Controls/ControlDrawingAreaGtk.o $(HUI_DIR)/Controls/ControlGroupGtk.o \
 $(HUI_DIR)/Controls/ControlLabelGtk.o $(HUI_DIR)/Controls/ControlListViewGtk.o \
 $(HUI_DIR)/Controls/ControlMultilineEditGtk.o $(HUI_DIR)/Controls/ControlProgressBarGtk.o \
 $(HUI_DIR)/Controls/ControlRadioButtonGtk.o $(HUI_DIR)/Controls/ControlToggleButtonGtk.o \
 $(HUI_DIR)/Controls/ControlSpinButtonGtk.o $(HUI_DIR)/Controls/ControlSliderGtk.o \
 $(HUI_DIR)/Controls/ControlTabControlGtk.o $(HUI_DIR)/Controls/ControlTreeViewGtk.o \
 $(HUI_DIR)/Controls/ControlExpanderGtk.o $(HUI_DIR)/Controls/ControlScrollerGtk.o \
 $(HUI_DIR)/Controls/ControlPanedGtk.o $(HUI_DIR)/Controls/ControlSeparatorGtk.o \
 $(HUI_DIR)/Controls/ControlRevealerGtk.o \
 $(HUI_DIR)/Controls/ToolItemButtonGtk.o $(HUI_DIR)/Controls/ToolItemMenuButtonGtk.o \
 $(HUI_DIR)/Controls/ToolItemSeparatorGtk.o $(HUI_DIR)/Controls/ToolItemToggleButtonGtk.o \
 $(HUI_DIR)/Controls/MenuItemGtk.o $(HUI_DIR)/Controls/MenuItemSubmenuGtk.o \
 $(HUI_DIR)/Controls/MenuItemSeparatorGtk.o $(HUI_DIR)/Controls/MenuItemToggleGtk.o \
 $(HUI_DIR)/Application.o
HUI_CXXFLAGS =  `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)


$(HUI_BIN) : $(HUI_OBJ)
	rm -f $@
	ar cq $@ $(HUI_OBJ)

$(HUI_DIR)/hui.o : $(HUI_DIR)/hui.cpp
	$(CPP) -c $(HUI_DIR)/hui.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/main_gtk.o : $(HUI_DIR)/main_gtk.cpp
	$(CPP) -c $(HUI_DIR)/main_gtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/main_win.o : $(HUI_DIR)/main_win.cpp
	$(CPP) -c $(HUI_DIR)/main_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Application.o : $(HUI_DIR)/Application.cpp
	$(CPP) -c $(HUI_DIR)/Application.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/common_dlg.o : $(HUI_DIR)/common_dlg.cpp
	$(CPP) -c $(HUI_DIR)/common_dlg.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/common_dlg_gtk.o : $(HUI_DIR)/common_dlg_gtk.cpp
	$(CPP) -c $(HUI_DIR)/common_dlg_gtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/common_dlg_win.o : $(HUI_DIR)/common_dlg_win.cpp
	$(CPP) -c $(HUI_DIR)/common_dlg_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/error.o : $(HUI_DIR)/error.cpp
	$(CPP) -c $(HUI_DIR)/error.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/clipboard.o : $(HUI_DIR)/clipboard.cpp
	$(CPP) -c $(HUI_DIR)/clipboard.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/language.o : $(HUI_DIR)/language.cpp
	$(CPP) -c $(HUI_DIR)/language.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/input.o : $(HUI_DIR)/input.cpp
	$(CPP) -c $(HUI_DIR)/input.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/resource.o : $(HUI_DIR)/resource.cpp
	$(CPP) -c $(HUI_DIR)/resource.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/utility.o : $(HUI_DIR)/utility.cpp
	$(CPP) -c $(HUI_DIR)/utility.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Config.o : $(HUI_DIR)/Config.cpp
	$(CPP) -c $(HUI_DIR)/Config.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Menu.o : $(HUI_DIR)/Menu.cpp
	$(CPP) -c $(HUI_DIR)/Menu.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/MenuGtk.o : $(HUI_DIR)/MenuGtk.cpp
	$(CPP) -c $(HUI_DIR)/MenuGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/MenuWin.o : $(HUI_DIR)/MenuWin.cpp
	$(CPP) -c $(HUI_DIR)/MenuWin.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/PainterGtk.o : $(HUI_DIR)/PainterGtk.cpp
	$(CPP) -c $(HUI_DIR)/PainterGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Panel.o : $(HUI_DIR)/Panel.cpp
	$(CPP) -c $(HUI_DIR)/Panel.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Timer.o : $(HUI_DIR)/Timer.cpp
	$(CPP) -c $(HUI_DIR)/Timer.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Window.o : $(HUI_DIR)/Window.cpp
	$(CPP) -c $(HUI_DIR)/Window.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/WindowGtk.o : $(HUI_DIR)/WindowGtk.cpp
	$(CPP) -c $(HUI_DIR)/WindowGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/WindowWin.o : $(HUI_DIR)/WindowWin.cpp
	$(CPP) -c $(HUI_DIR)/WindowWin.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Toolbar.o : $(HUI_DIR)/Toolbar.cpp
	$(CPP) -c $(HUI_DIR)/Toolbar.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/ToolbarGtk.o : $(HUI_DIR)/ToolbarGtk.cpp
	$(CPP) -c $(HUI_DIR)/ToolbarGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/ToolbarWin.o : $(HUI_DIR)/ToolbarWin.cpp
	$(CPP) -c $(HUI_DIR)/ToolbarWin.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/window_control.o : $(HUI_DIR)/window_control.cpp
	$(CPP) -c $(HUI_DIR)/window_control.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/window_control_gtk.o : $(HUI_DIR)/window_control_gtk.cpp
	$(CPP) -c $(HUI_DIR)/window_control_gtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/window_control_win.o : $(HUI_DIR)/window_control_win.cpp
	$(CPP) -c $(HUI_DIR)/window_control_win.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/Control.o : $(HUI_DIR)/Controls/Control.cpp
	$(CPP) -c $(HUI_DIR)/Controls/Control.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlButtonGtk.o : $(HUI_DIR)/Controls/ControlButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlCheckBoxGtk.o : $(HUI_DIR)/Controls/ControlCheckBoxGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlCheckBoxGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlComboBoxGtk.o : $(HUI_DIR)/Controls/ControlComboBoxGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlComboBoxGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlDrawingAreaGtk.o : $(HUI_DIR)/Controls/ControlDrawingAreaGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlDrawingAreaGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlEditGtk.o : $(HUI_DIR)/Controls/ControlEditGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlEditGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlGridGtk.o : $(HUI_DIR)/Controls/ControlGridGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlGridGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlGroupGtk.o : $(HUI_DIR)/Controls/ControlGroupGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlGroupGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlLabelGtk.o : $(HUI_DIR)/Controls/ControlLabelGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlLabelGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlListViewGtk.o : $(HUI_DIR)/Controls/ControlListViewGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlListViewGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlMultilineEditGtk.o : $(HUI_DIR)/Controls/ControlMultilineEditGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlMultilineEditGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlProgressBarGtk.o : $(HUI_DIR)/Controls/ControlProgressBarGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlProgressBarGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlRadioButtonGtk.o : $(HUI_DIR)/Controls/ControlRadioButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlRadioButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlSliderGtk.o : $(HUI_DIR)/Controls/ControlSliderGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlSliderGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlSpinButtonGtk.o : $(HUI_DIR)/Controls/ControlSpinButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlSpinButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlTabControlGtk.o : $(HUI_DIR)/Controls/ControlTabControlGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlTabControlGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlColorButtonGtk.o : $(HUI_DIR)/Controls/ControlColorButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlColorButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlToggleButtonGtk.o : $(HUI_DIR)/Controls/ControlToggleButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlToggleButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlTreeViewGtk.o : $(HUI_DIR)/Controls/ControlTreeViewGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlTreeViewGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlExpanderGtk.o : $(HUI_DIR)/Controls/ControlExpanderGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlExpanderGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlScrollerGtk.o : $(HUI_DIR)/Controls/ControlScrollerGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlScrollerGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlPanedGtk.o : $(HUI_DIR)/Controls/ControlPanedGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlPanedGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlSeparatorGtk.o : $(HUI_DIR)/Controls/ControlSeparatorGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlSeparatorGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlRevealerGtk.o : $(HUI_DIR)/Controls/ControlRevealerGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlRevealerGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlToolItemButtonGtk.o : $(HUI_DIR)/Controls/ControlToolItemButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlToolItemButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlToolItemMenuButtonGtk.o : $(HUI_DIR)/Controls/ControlToolItemMenuButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlToolItemMenuButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlToolItemSeparatorGtk.o : $(HUI_DIR)/Controls/ControlToolItemSeparatorGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlToolItemSeparatorGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlToolItemToggleButtonGtk.o : $(HUI_DIR)/Controls/ControlToolItemToggleButtonGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlToolItemToggleButtonGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlMenuItemGtk.o : $(HUI_DIR)/Controls/ControlMenuItemGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlMenuItemGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlMenuItemSubmenuGtk.o : $(HUI_DIR)/Controls/ControlMenuItemSubmenuGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlMenuItemSubmenuGtk.cpp -o $@ $(HUI_CXXFLAGS)

$(HUI_DIR)/Controls/ControlMenuItemToggleGtk.o : $(HUI_DIR)/Controls/ControlMenuItemToggleGtk.cpp
	$(CPP) -c $(HUI_DIR)/Controls/ControlMenuItemToggleGtk.cpp -o $@ $(HUI_CXXFLAGS)


