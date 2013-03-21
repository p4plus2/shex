#ifndef ABSTRACT_DIALOG_H
#define ABSTRACT_DIALOG_H

#include <QDialog>

class hex_editor;

class abstract_dialog : public QDialog
{
		Q_OBJECT
	public:
		explicit abstract_dialog();
		void set_active_editor(hex_editor *editor);
		virtual QString id() = 0;
		
	signals:
		
	public slots:
		virtual void refresh();
		
	protected:
		hex_editor *active_editor;
		
};

#endif // ABSTRACT_DIALOG_H
