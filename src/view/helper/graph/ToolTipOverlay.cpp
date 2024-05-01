#include "ToolTipOverlay.h"
#include "SceneGraph.h"
#include "../Drawing.h"
#include "../../ColorScheme.h"
#include "../../../lib/image/Painter.h"

ToolTipOverlay::ToolTipOverlay() {
	align.dz = 9999;
}

void ToolTipOverlay::on_draw(Painter *p) {
	p->set_font_size(theme.FONT_SIZE);
	p->set_line_width(theme.LINE_WIDTH);

	// hover/tool tip?
	string tip;
	if (graph()->hover.node)
		tip = graph()->hover.node->get_tip();
	if (tip.num > 0) {
		p->set_font("", theme.FONT_SIZE, true, false);
		draw_cursor_hover(p, tip, graph()->m, area);
		p->set_font("", theme.FONT_SIZE, false, false);
	}

	// general hint (full line at bottom)
	/*tip = view->mode->get_tip();
	if (tip.num > 0)
		draw_boxed_str(p, {view->song_area().center().x, area.y2 - 50}, tip, theme.text_soft1, theme.background_track_selected, TextAlign::CENTER);*/
}
