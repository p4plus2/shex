#include "select_range_dialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>

select_range_dialog::select_range_dialog(QWidget *parent) :
        QDialog(parent)
{
	start_label = new QLabel("&Starting SNES offset: ", this);
	start_input = new QLineEdit(this);
	end_label = new QLabel("&Ending SNES offset: ", this);
	end_input = new QLineEdit(this);
	absolute = new QRadioButton("Absolute offsets", this);
	relative = new QRadioButton("Relative offsets", this);
	select_range = new QPushButton("Select range");
	close = new QPushButton("Close");
	
	connect(close, SIGNAL(clicked()), this, SLOT(close()));
	connect(select_range, SIGNAL(clicked()), this, SLOT(range_entered()));
	
	start_label->setBuddy(start_input);
	end_label->setBuddy(end_input);
	absolute->setChecked(true);
	
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(start_label, 0, 0);
	layout->addWidget(start_input, 0, 1, 1, 2);
	layout->addWidget(end_label, 1, 0);
	layout->addWidget(end_input, 1, 1, 1, 2);
	layout->addWidget(absolute, 2, 0, 1, 3);
	layout->addWidget(relative, 3, 0, 1, 3);
	layout->addWidget(select_range, 4, 1);
	layout->addWidget(close, 4, 2);
	setLayout(layout);
}

void select_range_dialog::range_entered()
{
	int start_address = parse_input(start_input->text());
	int end_address = parse_input(end_input->text());
	if(start_address > 1 << 24 || end_address > 1 << 24){
		QMessageBox::warning(this, "Address error", "One or more addresses are out of the SNES's bounds!"
		                     "  Please check your addresses.");
	}else if(start_address < 0 || end_address < 0){
		return;
	}

	if(start_address > end_address){
		qSwap(start_address, end_address);
	}
	emit triggered(start_address, end_address, absolute->isChecked() ? true : false);
}

int select_range_dialog::parse_input(QString input)
{
	bool status;
	input.remove(QRegExp("[^0-9A-Fa-f]"));
	int address = input.toInt(&status, 16);
	if(address > 1 << 24){
		status = false;
	}else if(input.isEmpty()){
		return -1;
	}
	
	return address;
}
