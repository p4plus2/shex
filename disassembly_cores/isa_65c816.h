#ifndef ISA_65C816_H
#define ISA_65C816_H

#include <QCheckBox>
#include <QSet>
#include <QGridLayout>
#include "disassembler_core.h"

class isa_65c816 : public disassembler_core
{
		Q_OBJECT
	public:
		explicit isa_65c816(QObject *parent = 0);
		QString disassemble(int start, int end, const ROM_buffer *buffer);
		QGridLayout *core_layout();
		static QString id(){ return "65c186"; }
		
	signals:
		void A_changed(bool);
		void I_changed(bool);
		
	public slots:
		inline void toggle_A(bool state){ A_state = state; }
		inline void toggle_I(bool state){ I_state = state; }
		inline void toggle_error_stop(bool state){ error_stop = state; }

		
	private:
		bool A_state = false;
		bool I_state = false;
		bool error_stop = false;
		//These will get parented to a layout later
		QCheckBox *set_A = new QCheckBox("16 bit A");
		QCheckBox *set_I = new QCheckBox("16 bit I");
		QCheckBox *stop = new QCheckBox("Stop on unlikely");
		static const QList<disassembler_core::opcode> opcode_list;
		static const QSet<unsigned char> A_16_list;
		static const QSet<unsigned char> I_16_list;

		static const QSet<unsigned char> unlikely;
		
		static const QSet<unsigned char> branch_list;
		static const QSet<unsigned char> jump_list;
};

#endif // ISA_65C816_H
