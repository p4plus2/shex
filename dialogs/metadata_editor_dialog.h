#ifndef METADATA_EDITOR_DIALOG_H
#define METADATA_EDITOR_DIALOG_H

#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

#include "abstract_dialog.h"
#include "hex_editor.h"

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
		QLabel *region_label = new QLabel("Cart region: ", this);
		QLabel *name_label = new QLabel("Cart name: ", this);
		QLabel *header_labels[ROM_metadata::HEADER_COUNT];
		QLabel *checksum_labels[ROM_metadata::CHECKSUM_COUNT];
		QLabel *vector_labels[ROM_metadata::VECTOR_COUNT];
		
		QLabel *current_mapper_label = new QLabel("", this);
		QLabel *current_chips_label = new QLabel("", this);
		QLabel *current_region_label = new QLabel("", this);
		QLabel *current_name_label = new QLabel("", this);
		QLabel *current_header_labels[ROM_metadata::HEADER_COUNT];
		QLabel *current_checksum_labels[ROM_metadata::CHECKSUM_COUNT];
		QLabel *current_vector_labels[ROM_metadata::VECTOR_COUNT];
		
		QPushButton *apply = new QPushButton("Apply", this);
		QPushButton *cancel = new QPushButton("Cancel", this);
		
		QString calculate_chips();
};

#endif // METADATA_EDITOR_DIALOG_H
