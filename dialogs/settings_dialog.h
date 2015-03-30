#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include "abstract_dialog.h"
#include "settings_manager.h"

class settings_dialog : public abstract_dialog
{
		Q_OBJECT
	public:
		explicit settings_dialog(QWidget *parent);
		
	public slots:
		void refresh();
		void apply();
		
	private:
		settings_manager settings;
		
		QPushButton *refresh_button = new QPushButton("Refresh", this);
		QPushButton *apply_button = new QPushButton("Apply", this);
};

#endif // SETTINGS_DIALOG_H
