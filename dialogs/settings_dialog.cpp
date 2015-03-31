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

#include <QFontDatabase>
void settings_dialog::apply()
{
	QFontDatabase database;
	qDebug() << QFont::AnyStyle;
	qDebug() << QFont::SansSerif;
	qDebug() << QFont::Serif;
	qDebug() << QFont::TypeWriter;
	qDebug() << QFont::OldEnglish;
	qDebug() << QFont::Monospace;
	qDebug() << QFont::Fantasy;
	qDebug() << QFont::Cursive;
	qDebug() << QFont::System;
	
	for(const auto &family : database.families()){
		QFont font(family);
		QFontInfo info(font);
		qDebug() << family << info.styleHint();
		
		if(info.styleHint() == QFont::TypeWriter){
			qDebug() << family;
		}
	}
}
