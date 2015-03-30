#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include "abstract_dialog.h"

class settings_dialog : public abstract_dialog
{
		Q_OBJECT
	public:
		explicit settings_dialog(QWidget *parent);
};

#endif // SETTINGS_DIALOG_H
