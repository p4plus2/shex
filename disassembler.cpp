#include "disassembler.h"
#include "debug.h"
#include "disassembly_cores/isa_65c816.h"
#include "disassembly_cores/isa_spc700.h"
#include "events/event_types.h"
#include "utility.h"

disassembler::disassembler(QWidget *parent) :
        QPlainTextEdit(parent)
{
	cores.insert(isa_65c816::id(), new isa_65c816(this));
	cores.insert(isa_spc700::id(), new isa_spc700(this));
        for(auto i = cores.begin(); i != cores.end(); i++){
		disassembler_cores->addItem(i.key());
        }
	if(!display){
		disassembler_cores->hide();
		hide();
		core_layout->hide();
	}
	setReadOnly(true);
	connect(disassembler_cores, resolve<int>::from(&QComboBox::activated), this, &disassembler::update_core_layout);
}

void disassembler::disassemble(selection selection_area, const ROM_buffer *buffer)
{
	setPlainText(active_core()->disassemble(selection_area, buffer));
	show();
	toggle_display(true);
}

void disassembler::toggle_display(bool state) { 
	if(state == display){
		return;
	}
	setVisible(state);
	disassembler_cores->setVisible(state);
	core_layout->setVisible(state);
	display = state;
	layout_adjust();
}

// This method makes me very sad.
void disassembler::update_core_layout(int a)
{
	Q_UNUSED(a);
	QLayoutItem *child;
	while ((child = core_layout->layout()->takeAt(0))) {
		child->widget()->setParent(0);
		delete child;
	}
	delete core_layout;
	core_layout = new QWidget(this);
	box->addWidget(core_layout);
	core_layout->setLayout(active_core()->core_layout());
}

QVBoxLayout *disassembler::get_layout()
{	
	box->addWidget(disassembler_cores);
	box->addWidget(this);
	box->addWidget(core_layout);
	core_layout->setLayout(active_core()->core_layout());
	return box;
}

bool disassembler::event(QEvent *event)
{
	if(event->type() != (QEvent::Type)DISA_PANEL_EVENT){
		return QWidget::event(event);
	}
	switch(((disa_panel_event *)event)->sub_type()){
		case DISA_TOGGLE_DISPLAY:
			toggle_display(!display);
			return true;
		default:
			qDebug() << "Bad event" << ((editor_event *)event)->sub_type();
			return false;
	}
}

void disassembler::layout_adjust()
{
        QWidget *parent = parentWidget();
        while(parent){
		int height = parent->height();
		parent->setUpdatesEnabled(false);
		parent->adjustSize();
		parent->resize(parent->width(), height);
		parent->setUpdatesEnabled(true);
		parent->repaint();
		parent = parent->parentWidget();
	}
}

bool disassembler::display = false;
