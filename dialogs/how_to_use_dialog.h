#ifndef HOW_TO_USE_H
#define HOW_TO_USE_H

#include <QObject>

#include "abstract_dialog.h"

class how_to_use_dialog : public abstract_dialog
{
	public:
		how_to_use_dialog(QWidget *parent);
		
	private:
		void add_text_block(QString title, QString body, QVBoxLayout *layout);
};

#endif // HOW_TO_USE_H
