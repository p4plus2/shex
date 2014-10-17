#ifndef ISA_SPC700_H
#define ISA_SPC700_H

#include "disassembler_core.h"
#include <QCheckBox>
#include <QList>
#include <QSet>

class isa_spc700 : public disassembler_core
{
		Q_OBJECT
	public:
		explicit isa_spc700(QObject *parent = 0);
		QGridLayout *core_layout();
		static QString id(){ return "SPC700"; }

	public slots:
		inline void toggle_error_stop(bool state){ error_stop = state; }

	protected:
		QString decode_name_arg(const char arg, int &size);
		opcode get_opcode(int op);
		bool abort_unlikely(int op);
		void update_state();
	private:		
		bool error_stop = false;
		//These will get parented to a layout later
		QCheckBox *stop = new QCheckBox("Stop on unlikely");
		static const QList<disassembler_core::opcode> opcode_list;
		static const QSet<unsigned char> unlikely;
		
};

#endif // ISA_SPC700_H
