#ifndef DISASSEMBLER_CORE_H
#define DISASSEMBLER_CORE_H
#include <QObject>
#include <QGridLayout>

class disassembler_core : public QObject
{
	Q_OBJECT
	public:
		struct opcode{
			QString name;
			int size;
		};
		using QObject::QObject;
		virtual QGridLayout *core_layout();
		virtual QString disassemble(QByteArray *data) = 0;
};

#endif // DISASSEMBLER_CORE_H
