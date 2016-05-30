#ifndef ISA_GSU_H
#define ISA_GSU_H

#include <QCheckBox>
#include <QMap>
#include <QSet>
#include <QGridLayout>

#include "disassembler_core.h"

class isa_gsu : public disassembler_core
{
		friend class isa_gsu_ui;
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
		void update_state();
		void set_flags(bookmark_data::types type){ Q_UNUSED(type); }
	private:		
		int alt_state = 0;
		bool error_stop = false;
		
		static const QList<disassembler_core::opcode> opcode_list;
		static const QMap<unsigned char, disassembler_core::opcode> alt1;
		static const QMap<unsigned char, disassembler_core::opcode> alt2;
		static const QMap<unsigned char, disassembler_core::opcode> alt3;
		static const QSet<unsigned char> unlikely;
		
		const QMap<unsigned char, disassembler_core::opcode> *alt_states[4] = {nullptr, &alt1, &alt2, &alt3};
};

class isa_gsu_ui : public disassembler_core_ui
{
		Q_OBJECT
		friend class isa_gsu;
	public:
		explicit isa_gsu_ui(QObject *parent);
		~isa_gsu_ui();
		QGridLayout *core_layout();
		
	public slots:
		void change_alt(const QString &text){ ((isa_gsu *)disassembler())->alt_state = text.toInt(); }
		void toggle_error_stop(bool state){ ((isa_gsu *)disassembler())->error_stop = state; }

	private:		
		QLineEdit *set_alt = new QLineEdit();
		QCheckBox *stop = new QCheckBox("Stop on unlikely");
};

class isa_gsu_options : public disassembler_core_options
{
	using disassembler_core_options::disassembler_core_options;
};

struct isa_gsu_factory : public disassembler_core_factory
{
	using disassembler_core_factory::disassembler_core_factory;
	virtual disassembler_core *get_core(QObject *parent) { return new isa_gsu(parent); }
	virtual disassembler_core_ui *get_ui(QObject *parent) { return new isa_gsu_ui(parent); }
	virtual disassembler_core_options *get_options(QObject *parent) { return new isa_gsu_options(parent); }
};

static isa_gsu_factory factory("isa_gsu");
#endif // ISA_GSU_H
