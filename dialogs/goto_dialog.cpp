#include "goto_dialog.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QDebug>

goto_dialog::goto_dialog(QWidget *parent) :
        QDialog(parent)
{
	connect(close, SIGNAL(clicked()), this, SLOT(close()));
	connect(goto_offset, SIGNAL(clicked()), this, SLOT(address_entered()));
	
	label->setBuddy(offset_input);
	absolute->setChecked(true);
	
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(label, 0, 0);
	layout->addWidget(offset_input, 0, 1, 1, 2);
	layout->addWidget(absolute, 1, 0, 1, 3);
	layout->addWidget(relative, 2, 0, 1, 3);
	layout->addWidget(goto_offset, 3, 1);
	layout->addWidget(close, 3, 2);
	setLayout(layout);
}

void goto_dialog::address_entered()
{
	bool status;
	QString input = offset_input->text().remove(QRegExp("[^0-9A-Fa-f]"));
	int address = input.toInt(&status, 16);
	if(address > 1 << 24){
		status = false;
	}else if(input.isEmpty()){
		return;
	}
	if(status){
		emit triggered(address, absolute->isChecked() ? true : false);
	}else{
		QMessageBox::warning(this, "Address error", "Address out of SNES bounds!  Please check your address.");
	}
}
