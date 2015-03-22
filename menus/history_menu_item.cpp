#include "history_menu_item.h"
#include <QUndoGroup>
#include "hex_editor.h"

history_menu_item::history_menu_item(QString t, QEvent *e, QKeySequence hotkey, menu_manager *parent, QUndoGroup *g)
        : abstract_menu_item(t, e, nullptr, hotkey, parent, false)
{
	prefix = t;
	
	if(text() == "U&ndo"){
		connect(g, &QUndoGroup::canUndoChanged, this, &history_menu_item::setEnabled);
		connect(g, &QUndoGroup::undoTextChanged, this, &history_menu_item::set_prefix);
		connect(this, &history_menu_item::triggered, g, &QUndoGroup::undo);
	}else{
		connect(g, &QUndoGroup::canRedoChanged, this, &history_menu_item::setEnabled);
		connect(g, &QUndoGroup::redoTextChanged, this, &history_menu_item::set_prefix);
		connect(this, &history_menu_item::triggered, g, &QUndoGroup::redo);
	}
}

void history_menu_item::set_prefix(const QString &t)
{
	setText(prefix + " " + t);

}
