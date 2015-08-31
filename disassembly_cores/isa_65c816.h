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
		~isa_65c816();
		QGridLayout *core_layout();
		static QString id(){ return "65c816"; }
		
	signals:
		void A_changed(bool);
		void I_changed(bool);
		
	public slots:
		void toggle_A(bool state){ A_state = state; }
		void toggle_I(bool state){ I_state = state; }
		void toggle_error_stop(bool state){ error_stop = state; }

	protected:
		QString decode_name_arg(const char arg, int &size);
		QString address_to_label(int address);
		QString format_data_value(int size, int value, bool is_pointer);
		template <typename V> QString label_op(int offset, int size, V validator);
		opcode get_opcode(int op);
		int get_base();
		bool abort_unlikely(int op);
		void update_state();
		void set_flags(bookmark_data::types type);
	private:		
		bool A_state = false;
		bool I_state = false;
		bool error_stop = false;
		QCheckBox *set_A = new QCheckBox("16 bit A");
		QCheckBox *set_I = new QCheckBox("16 bit I");
		QCheckBox *stop = new QCheckBox("Stop on unlikely");
		static const QList<disassembler_core::opcode> opcode_list;
		static const QSet<unsigned char> unlikely;
};

#endif // ISA_65C816_H
