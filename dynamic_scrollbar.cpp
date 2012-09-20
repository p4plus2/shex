#include "dynamic_scrollbar.h"
#include <QStyle>
#include <QStyleOptionSlider>
#include "qdebug.h"

dynamic_scrollbar::dynamic_scrollbar(QWidget *parent) :
        QScrollBar(parent)
{
	original_pagestep = pageStep();
	scroll_style = new scrollbar_style();
	setStyle(scroll_style);
	toggle_mode(false);
}

void dynamic_scrollbar::set_range(int value)
{
	setRange(0, value);
}

void dynamic_scrollbar::toggle_mode(bool m)
{
	mode = m;
	scroll_style->toggle(mode);
	setPageStep(mode ? 0 : original_pagestep);
	if(mode){
		setValue(maximum() / 2 - scroll_style->pixelMetric(QStyle::PM_ScrollBarSliderMin, 0, 0) / 2);
	}
}
