#ifndef BOOKMARK_DIALOG_H
#define BOOKMARK_DIALOG_H

#include <QWidget>
#include "abstract_dialog.h"

class bookmark_dialog : public abstract_dialog
{
		Q_OBJECT
	public:
		explicit bookmark_dialog();
		virtual QString id(){ return "bookmarks"; }
};


#endif // BOOKMARK_DIALOG_H
