#ifndef ABSTRACT_DIALOG_H
#define ABSTRACT_DIALOG_H

#include <QDialog>

#include "hex_editor.h"
#include "debug.h"

class abstract_dialog : public QDialog
{
		Q_OBJECT
	public:
		explicit abstract_dialog(QWidget *parent);
		void set_active_editor(hex_editor *editor);
		
	public slots:
		virtual void refresh();
		
	protected:
		hex_editor *active_editor;
		
		bool validate_address(int addr){ return active_editor->get_buffer()->validate_address(addr); }
		
};

#endif // ABSTRACT_DIALOG_H
