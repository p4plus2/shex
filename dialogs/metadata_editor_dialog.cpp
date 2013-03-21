#include "metadata_editor_dialog.h"
#include "debug.h"
#include "hex_editor.h"

#include <QGridLayout>

metadata_editor_dialog::metadata_editor_dialog()
{
	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(mapper_label, 0, 0);
	layout->addWidget(chips_label, 1, 0);
	
	layout->addWidget(current_mapper_label, 0, 1);
	layout->addWidget(current_chips_label, 1, 1);
	
	layout->addWidget(apply, 4, 1);
	layout->addWidget(cancel, 4, 2);
	setLayout(layout);
}

void metadata_editor_dialog::refresh()
{
	if(active_editor){
		ROM_buffer *buffer = active_editor->get_buffer();
		current_chips_label->setText(ROM_metadata::mapper_strings.at(buffer->get_mapper()));
	}
	abstract_dialog::refresh();
}
