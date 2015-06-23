#include "expand_rom_dialog.h"

expand_ROM_dialog::expand_ROM_dialog(QWidget *parent) : abstract_dialog(parent)
{
	QGridLayout *layout = new QGridLayout(this);
	QLabel *label = new QLabel("todo.");
	layout->addWidget(label, 0, 0);
	setLayout(layout);
}
