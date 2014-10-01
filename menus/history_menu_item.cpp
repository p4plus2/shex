#include "history_menu_item.h"
#include <QUndoGroup>
#include "hex_editor.h"

history_menu_item::history_menu_item(QString text, QString r, QString t, QKeySequence hotkey, QWidget *parent) :
        abstract_menu_item(text, parent)
{
	run = r;
	toggle = t;
	setShortcut(hotkey);
	prefix = text;
	connect(this, &history_menu_item::triggered, this, &history_menu_item::activated);
}


void history_menu_item::connect_to_widget(QUndoGroup *group)
{
	if(text() == "U&ndo"){
		connect(group, &QUndoGroup::canUndoChanged, this, &history_menu_item::setEnabled);
		connect(group, &QUndoGroup::undoTextChanged, this, &history_menu_item::set_prefix);
		connect(this, &history_menu_item::triggered, group, &QUndoGroup::undo);
	}else{
		connect(group, &QUndoGroup::canRedoChanged, this, &history_menu_item::setEnabled);
		connect(group, &QUndoGroup::redoTextChanged, this, &history_menu_item::set_prefix);
		connect(this, &history_menu_item::triggered, group, &QUndoGroup::redo);
	}
	
}

void history_menu_item::connect_to_widget(hex_editor *editor)
{
	connect(this, SIGNAL(run_history_update(bool)), editor, run.toUtf8().data());
}

void history_menu_item::set_prefix(const QString &t)

{
	setText(prefix + " " + t);

}
