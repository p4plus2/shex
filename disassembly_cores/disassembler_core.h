#ifndef DISASSEMBLER_CORE_H
#define DISASSEMBLER_CORE_H

#include <QGridLayout>

#include "rom_buffer.h"
#include "selection.h"

class disassembler_core : public QObject
{
	Q_OBJECT
	public:
		struct opcode{
			QString name;
		};
		using QObject::QObject;
		virtual QGridLayout *core_layout() = 0;
		virtual QString disassemble(selection selection_area, const ROM_buffer *b);
		
	protected:
		QByteArray data;
		selection region;
		const ROM_buffer *buffer;
		int delta;
		
		QString add_label(int destination);
		void add_mnemonic(int destination, QString mnemonic);
		QString disassembly_text();
		void reset();
		void decode_name_args(QString &name);
		QString get_hex(int n, int bytes);
		unsigned int get_operand(int n);
		
		virtual QString decode_name_arg(const char arg, int &size) = 0;
		virtual QString address_to_label(int address) = 0;
		virtual opcode get_opcode(int op) = 0;
		virtual int get_base() = 0;
		virtual bool abort_unlikely(int op) = 0;
		virtual void update_state() = 0;
		
	private:
		struct block{
			QString label = "";
			QString mnemonic = "";
		};

		QMap<int, block> disassembly_list;
		int label_id;
		
		QString make_table(QByteArray &data, int start, int size, int width, bool packed);
};

#endif // DISASSEMBLER_CORE_H
