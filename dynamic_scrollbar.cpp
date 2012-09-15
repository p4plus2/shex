#include "dynamic_scrollbar.h"

dynamic_scrollbar::dynamic_scrollbar(QWidget *parent) :
        QScrollBar(parent)
{
}

void dynamic_scrollbar::set_range(int value)
{
	setRange(0, value);
}
