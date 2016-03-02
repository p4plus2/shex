#ifndef DISASSEMBLER_CORE_H
#define DISASSEMBLER_CORE_H

#include <QGridLayout>

#include "rom_buffer.h"
#include "selection.h"

class disassembler_core
{
	public:
		explicit disassembler_core(QObject *p = 0){ parent = p; }
		struct opcode{
			enum operand_hints{
				NONE,
				WORD_ADDRESS_RAM,
				LONG_ADDRESS_RAM,
				WORD_ADDRESS_ROM,
				LONG_ADDRESS_ROM,
				WORD_ADDRESS_ANY,
				LONG_ADDRESS_ANY,
				WORD_JUMP,
				LONG_JUMP,
				INDIRECT_JUMP,
				BRANCH,
				FLAGS,
				CONST,
				INDEX,
				MOVE
			};
			QString name;
			operand_hints hint;
		};
		virtual QString disassemble(selection selection_area, const ROM_buffer *b);
		virtual ~disassembler_core(){}
		
	protected:
		QObject *parent;
		QByteArray data;
		selection region;
		const ROM_buffer *buffer;
		int delta;
		QString error;
		
		QString add_label(int destination, QString prefix = "");
		QString disassembly_text();
		void reset();
		void decode_name_args(QString &name);
		unsigned int get_instruction();
		unsigned int get_operand(int n);
		bool in_range(int address);
		
		virtual QString decode_name_arg(const char arg, int &size) = 0;
		virtual QString address_to_label(int address) = 0;
		virtual QString format_data_value(int size, int value, bool is_pointer) = 0;
		virtual opcode get_opcode(int op) = 0;
		virtual int get_base() = 0;
		virtual bool is_unlikely_opcode(int op) = 0;
		virtual bool is_semiunlikely_opcode(int op){ Q_UNUSED(op); return false; }
		virtual bool is_codeflow_opcode(int op){ Q_UNUSED(op); return false; }
		virtual bool is_unlikely_operand(){ return false; }
		virtual bool is_stateful_opcode(int op){ Q_UNUSED(op); return false; }
		virtual void update_state() = 0;
		virtual void set_flags(bookmark_data::types flags) = 0;
		virtual void push_state(){}
		virtual void pop_state(){}
		virtual void reset_stack(){}
		
	private:		
		struct block{
			enum data_format{
				CODE = 0,
				DATA_PACKED = 1,
				DATA_PACKED_BYTE = DATA_PACKED,
				DATA_PACKED_END = 4,
				DATA_UNPACKED = 5,
				DATA_UNPACKED_WORD = DATA_UNPACKED + 1,
				DATA_UNPACKED_END = 9,
				DATA_STRING = 10
			};
			QString label = "";
			QString data = "";
			data_format format;
		};

		QMap<int, block> disassembly_list;
		int label_id;
		bool previous_codeflow = false;
		bool inspecting_data = false;
		
		void add_data(int destination, QString data, block::data_format format);
		void disassemble_table(const bookmark_data &bookmark);
		void disassemble_rats();
		void disassemble_code();
		bool disassemble_data();
};

class disassembler_core_ui : public QObject
{
	public:
		using QObject::QObject;
		~disassembler_core_ui(){ delete core; }
		
		virtual QGridLayout *core_layout() = 0;
		QString disassemble(selection selection_area, const ROM_buffer *b);
	
	protected:
		disassembler_core *disassembler(){ return core; }
		void set_disassembler(disassembler_core *c){ core = c; }
		
	private:
		disassembler_core *core;
};

#endif // DISASSEMBLER_CORE_H
