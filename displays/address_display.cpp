#include "address_display.h"
#include <QPainter>

void address_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	set_painter_font(painter);
}
