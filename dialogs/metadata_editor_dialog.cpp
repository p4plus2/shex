#include "metadata_editor_dialog.h"
#include "debug.h"

#include <QGridLayout>

metadata_editor_dialog::metadata_editor_dialog()
{
	QGridLayout *layout = new QGridLayout(this);
	
#define add_widget(ITEM, ROW) \
	layout->addWidget(ITEM ## _label, ROW, 0); \
	layout->addWidget(current_## ITEM ##_label, ROW, 1)
	
	add_widget(mapper, 0);
	add_widget(chips, 1);
	add_widget(region, 2);
	add_widget(name, 3);
	
#undef add_widget
#define label_init(ENUM, NAME) \
	for(int i = 0; i < ROM_metadata::ENUM ##_COUNT; i++){ \
		NAME ##_labels[i] = new QLabel(ROM_metadata::NAME ##_strings[i].second + ":",this); \
		current_## NAME ##_labels[i] = new QLabel("", this); \
		layout->addWidget(NAME ## _labels[i], base_row+i, 0); \
		layout->addWidget(current_## NAME ##_labels[i], base_row+i, 1); \
	} \
	base_row += ROM_metadata::ENUM ##_COUNT
	
	int base_row = 4;
	label_init(HEADER, header);
	label_init(CHECKSUM, checksum);
	label_init(VECTOR, vector);
#undef label_init

	layout->addWidget(apply, base_row, 0);
	layout->addWidget(cancel, base_row, 1);
	setLayout(layout);
}

void metadata_editor_dialog::refresh()
{
	if(active_editor){
		ROM_buffer *buffer = active_editor->get_buffer();
		current_mapper_label->setText(ROM_metadata::mapper_strings[buffer->get_mapper()].second);
		current_chips_label->setText(calculate_chips());
		current_region_label->setText(ROM_metadata::region_strings[buffer->get_cart_region()].second);
		current_name_label->setText(buffer->get_cart_name());
		
		for(int i = 0; i < ROM_metadata::HEADER_COUNT; i++){
			QString field = "$" + QString::number(buffer->get_header_field(
			                ROM_metadata::header_strings[i].first),16).toUpper().leftJustified(2,'0');
			current_header_labels[i]->setText(field);
		}
		for(int i = 0; i < ROM_metadata::CHECKSUM_COUNT; i++){
			QString field = "$" + QString::number(buffer->get_header_field(
			                ROM_metadata::checksum_strings[i].first),16).toUpper().leftJustified(4,'0');
			current_checksum_labels[i]->setText(field);
		}
		for(int i = 0; i < ROM_metadata::VECTOR_COUNT; i++){
			QString field = "$"+ QString::number(buffer->get_vector(
			                ROM_metadata::vector_strings[i].first), 16).toUpper().leftJustified(4,'0');;
			current_vector_labels[i]->setText(field);
		}
		
	}
	abstract_dialog::refresh();
}

QString metadata_editor_dialog::calculate_chips()
{
	QString chips;
	ROM_buffer *buffer = active_editor->get_buffer();
	for(int i = 0; i < ROM_metadata::NO_CHIPS; i++){
		if(buffer->has_chip((ROM_metadata::cart_chips)i)){
			if(chips != ""){
				chips.append(", ");
			}
			chips.append(buffer->chip_strings[i].second);
		}
	}
	if(chips == ""){
		chips.append(buffer->chip_strings[ROM_metadata::NO_CHIPS].second);
	}

	return chips;
}
