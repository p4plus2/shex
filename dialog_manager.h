#ifndef DIALOG_MANAGER_H
#define DIALOG_MANAGER_H

#include "dialogs/goto_dialog.h"
#include "dialogs/select_range_dialog.h"
#include "dialogs/expand_rom_dialog.h"
#include "dialogs/metadata_editor_dialog.h"

#include "debug.h"

#include <QObject>

class hex_editor;

class dialog_manager : public QObject
{
		Q_OBJECT
	public:
		explicit dialog_manager(QObject *parent = 0);
		~dialog_manager();
		
		void connect_to_editor(hex_editor *editor);
		
	signals:
		
	public slots:
		inline void show_goto_dialog() { raise_dialog(goto_window); }
		inline void show_select_range_dialog() { raise_dialog(select_range_window); }
		inline void show_expand_dialog(){ raise_dialog(expand_ROM_window); }
		inline void show_metadata_editor_dialog(){ raise_dialog(metadata_editor_window); qDebug() << "test";}
		
	private:
		goto_dialog *goto_window = new goto_dialog();
		select_range_dialog *select_range_window = new select_range_dialog();
		expand_ROM_dialog *expand_ROM_window = new expand_ROM_dialog();
		metadata_editor_dialog *metadata_editor_window = new metadata_editor_dialog();
		
		void raise_dialog(QDialog *dialog);
		
};

#endif // DIALOG_MANAGER_H
