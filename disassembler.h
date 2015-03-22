#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <QPlainTextEdit>
#include <QComboBox>
#include <QLayout>
#include <QMap>
#include "disassembly_cores/disassembler_core.h"
#include "debug.h"
#include "selection.h"

class disassembler : public QPlainTextEdit
{
		Q_OBJECT
	public:

		explicit disassembler(QWidget *parent = 0);
		QVBoxLayout *get_layout();
		
	public slots:
		void disassemble(selection selection_area, const ROM_buffer *buffer);
		void toggle_display(bool state);
		void update_core_layout(int a);
	
	protected:
		virtual bool event(QEvent *event);
		
	private:
		static bool display;
		void layout_adjust();
		disassembler_core *active_core(){ return cores[disassembler_cores->currentText()]; }
		
		QWidget *core_layout = new QWidget(this);
		QVBoxLayout *box = new QVBoxLayout();	
		QComboBox *disassembler_cores = new QComboBox(this);
		QMap<QString, disassembler_core *> cores;
};

#endif // DISASSEMBLER_H
