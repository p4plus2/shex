#include "how_to_use_dialog.h"

how_to_use_dialog::how_to_use_dialog(QWidget *parent) : abstract_dialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	add_text_block("Bookmarks:", 
	               "The bookmark system is designed to interact with the disassembler built into shex and "
	               "help track locations within a ROM.  To load and save bookmark files simply right click "
	               "on the bookmark list region and select the given option.  The drop down menu next to "
	               "description provides hints to the disassembler about how to format a code region. "
	               "The save format is json based for anybody wishing to parse it in external tools.",
	               layout);
	add_text_block("Comparison:",
	               "When comparisons are active only the primary file is editable.  Generating a patch "
	               "will create an assembler patch that can transform the original ROM into the secondary ROM.",
	               layout);
	add_text_block("Dynamic scrollbar:",
	               "The dynamic scroll bar, when active, acts as a auto scroll speed slider.  Dragging further "
	               "from the center point results in a faster scroll in that direction.",
	               layout);
	setLayout(layout);
}

void how_to_use_dialog::add_text_block(QString title, QString body, QVBoxLayout *layout)
{
	const int label_spacer = -5;
	const int block_spacer = 10;
	
	QLabel *body_label = new QLabel(body, this);
	body_label->setWordWrap(true);
	
	layout->addWidget(new QLabel(title, this));
	layout->addSpacing(label_spacer);
	
	layout->addWidget(body_label);
	layout->addSpacing(block_spacer);
}
