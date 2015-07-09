#ifndef ISA_GSU_H
#define ISA_GSU_H

#include <QCheckBox>
#include <QMap>
#include <QSet>
#include <QGridLayout>

#include "disassembler_core.h"

class isa_gsu : public disassembler_core
{
		Q_OBJECT
	public:
		explicit isa_gsu(QObject *parent);
		~isa_gsu();
		QGridLayout *core_layout();
		static QString id(){ return "gsu"; }
		
	public slots:
		inline void change_alt(const QString &text){ alt_state = text.toInt(); }
		inline void toggle_error_stop(bool state){ error_stop = state; }

	protected:
		QString decode_name_arg(const char arg, int &size);
		QString address_to_label(int address);
		template <typename V> QString label_op(int offset, int size, V validator);
		opcode get_opcode(int op);
		int get_base();
		bool abort_unlikely(int op);
		void update_state();
		void set_flags(bookmark_data::types type){ Q_UNUSED(type); }
	private:		
		int alt_state = 0;
		bool error_stop = false;
		QLineEdit *set_alt = new QLineEdit();
		QCheckBox *stop = new QCheckBox("Stop on unlikely");
		
		static const QList<disassembler_core::opcode> opcode_list;
		static const QMap<unsigned char, disassembler_core::opcode> alt1;
		static const QMap<unsigned char, disassembler_core::opcode> alt2;
		static const QMap<unsigned char, disassembler_core::opcode> alt3;
		static const QSet<unsigned char> unlikely;
		
		const QMap<unsigned char, disassembler_core::opcode> *alt_states[4] = {nullptr, &alt1, &alt2, &alt3};
};

#endif // ISA_GSU_H
