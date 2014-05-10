#ifndef DISASSEMBLER_CORE_H
#define DISASSEMBLER_CORE_H
#include <QObject>
#include <QMap>
#include <QGridLayout>
#include "../rom_buffer.h"

class disassembler_core : public QObject
{
	Q_OBJECT
	public:
		struct opcode{
			QString name;
			int size;
		};
		using QObject::QObject;
		virtual QGridLayout *core_layout() = 0;
		virtual QString disassemble(int start, int end, const ROM_buffer *buffer) = 0;
		
	protected:
		QString add_label(int destination);
		void add_mnemonic(int destination, QString mnemonic);
		QString disassembly_text();
		void reset();
		
	private:
		struct block{
			QString label = "";
			QString mnemonic = "";
		};

		QMap<int, block> disassembly_list;
		int label_id;
};

#endif // DISASSEMBLER_CORE_H
