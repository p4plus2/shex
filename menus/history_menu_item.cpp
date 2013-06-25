#include "history_menu_item.h"
#include <QUndoGroup>
#include <main_window.h>

history_menu_item::history_menu_item(QString text, QString r, QString t, QKeySequence hotkey, QWidget *parent) :
        abstract_menu_item(text, parent)
{
	run = r;
	toggle = t;
	setShortcut(hotkey);
	prefix = text;
}


void history_menu_item::connect_to_widget(QUndoGroup *group)
{
	if(text() == "U&ndo"){
		connect(group, SIGNAL(canUndoChanged(bool)), this, SLOT(setEnabled(bool)));
		connect(group, SIGNAL(undoTextChanged(QString)), this, SLOT(set_prefix(QString)));
		connect(this, SIGNAL(triggered()), group, SLOT(undo()));
	}else{
		connect(group, SIGNAL(canRedoChanged(bool)), this, SLOT(setEnabled(bool)));
		connect(group, SIGNAL(redoTextChanged(QString)), this, SLOT(set_prefix(QString)));
		connect(this, SIGNAL(triggered()), group, SLOT(redo()));
	}
	
}

void history_menu_item::connect_to_widget(main_window *window)
{
	connect(this, SIGNAL(triggered()), window, run.toAscii().data());
}


void history_menu_item::set_prefix(const QString &t)

{
	setText(prefix + " " + t);

}
