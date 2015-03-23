#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <QPlainTextEdit>
#include <QComboBox>
#include <QLayout>
#include <QMap>
#include "disassembly_cores/disassembler_core.h"
#include "debug.h"
#include "selection.h"
#include "abstract_panel.h"

class disassembler_panel : public QPlainTextEdit, public abstract_panel
{
		Q_OBJECT
	public:

		explicit disassembler_panel(QWidget *parent = 0);
		virtual QLayout *get_layout();
		
	public slots:
		void disassemble(selection selection_area, const ROM_buffer *buffer);
		void update_core_layout(int a);
		
	private:
		void layout_adjust();
		disassembler_core *active_core(){ return cores[disassembler_cores->currentText()]; }
		
		QWidget *core_layout = new QWidget(this);
		QVBoxLayout *box = new QVBoxLayout();	
		QComboBox *disassembler_cores = new QComboBox(this);
		QMap<QString, disassembler_core *> cores;
};

#endif // DISASSEMBLER_H
