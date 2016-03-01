#ifndef ISA_65C816_H
#define ISA_65C816_H

#include <QCheckBox>
#include <QSet>
#include <QGridLayout>
#include <QStack>

#include "disassembler_core.h"

class isa_65c816 : public disassembler_core
{
		friend class isa_65c816_ui;
	public:
		using disassembler_core::disassembler_core;
		
	protected:
		QString decode_name_arg(const char arg, int &size);
		QString address_to_label(int address);
		QString format_data_value(int size, int value, bool is_pointer);
		template <typename V> QString label_op(int offset, int size, V validator);
		opcode get_opcode(int op);
		int get_base();
		bool is_unlikely_opcode(int op);
		bool is_semiunlikely_opcode(int op);
		bool is_codeflow_opcode(int op);
		bool is_stateful_opcode(int op);
		bool is_unlikely_operand();
		void update_state();
		void set_flags(bookmark_data::types type);
		void push_state();
		void pop_state();
		void reset_stack();
		
	private:		
		bool A_state = false;
		bool I_state = false;
		bool error_stop = false;
		
		struct state{
			bool A_state;
			bool I_state;
		};
		
		QStack<state> state_stack;

		static const QList<disassembler_core::opcode> opcode_list;
		static const QSet<unsigned char> unlikely;
		static const QSet<unsigned char> semiunlikely;
		static const QSet<unsigned char> codeflow;
};

class isa_65c816_ui : public disassembler_core_ui
{
		Q_OBJECT
		friend class isa_65c816;
	public:
		explicit isa_65c816_ui(QObject *parent);
		~isa_65c816_ui();
		QGridLayout *core_layout();
		static QString id(){ return "65c816"; }
		
	public slots:
		void toggle_A(bool state){ ((isa_65c816 *)disassembler())->A_state = state; }
		void toggle_I(bool state){ ((isa_65c816 *)disassembler())->I_state = state; }
		void toggle_error_stop(bool state){ ((isa_65c816 *)disassembler())->error_stop = state; }

	private:		
		QCheckBox *set_A = new QCheckBox("16 bit A");
		QCheckBox *set_I = new QCheckBox("16 bit I");
		QCheckBox *stop = new QCheckBox("Stop on unlikely");
};

#endif // ISA_65C816_H
