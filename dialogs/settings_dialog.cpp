#include "settings_dialog.h"

settings_dialog::settings_dialog(QWidget *parent) : abstract_dialog(parent)
{
	QGridLayout *layout = new QGridLayout(this);

	layout->addWidget(refresh_button, 4, 1);
	layout->addWidget(apply_button, 4, 2);
	setLayout(layout);
	
	connect(refresh_button, &QPushButton::clicked, this, &settings_dialog::refresh);
	connect(apply_button, &QPushButton::clicked, this, &settings_dialog::apply);
}

void settings_dialog::refresh()
{
	
}

void settings_dialog::apply()
{

}
