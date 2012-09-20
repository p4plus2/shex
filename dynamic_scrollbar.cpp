#include "dynamic_scrollbar.h"
#include <QStyle>
#include <QStyleOptionSlider>
#include "qdebug.h"

dynamic_scrollbar::dynamic_scrollbar(QWidget *parent) :
        QScrollBar(parent)
{
	original_pagestep = pageStep();
	setStyle(new scrollbar_style);
	toggle_mode(false);
}

void dynamic_scrollbar::set_range(int value)
{
	setRange(0, value);
}

void dynamic_scrollbar::toggle_mode(bool m)
{
	mode = m;
	dynamic_cast<scrollbar_style *>(style())->toggle(mode);
	setPageStep(mode ? 0 : original_pagestep);
}
