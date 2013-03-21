#ifndef METADATA_EDITOR_DIALOG_H
#define METADATA_EDITOR_DIALOG_H

#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

#include "abstract_dialog.h"

class metadata_editor_dialog : public abstract_dialog
{
		Q_OBJECT
	public:
		explicit metadata_editor_dialog();
		virtual QString id(){ return "metadata_editor"; }
		virtual void refresh();
		
	signals:
		
	public slots:
		
	private:
		QLabel *mapper_label = new QLabel("Memory mapper: ", this);
		QLabel *chips_label = new QLabel("Enabled chips: ", this);
		
		QLabel *current_mapper_label = new QLabel("", this);
		QLabel *current_chips_label = new QLabel("", this);

		
		QPushButton *apply = new QPushButton("Apply", this);
		QPushButton *cancel = new QPushButton("Cancel", this);
		
};

#endif // METADATA_EDITOR_DIALOG_H
