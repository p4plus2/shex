#ifndef DYNAMIC_SCROLLBAR_H
#define DYNAMIC_SCROLLBAR_H

#include <QScrollBar>

class dynamic_scrollbar : public QScrollBar
{
		Q_OBJECT
	public:
		explicit dynamic_scrollbar(QWidget *parent = 0);
		
	signals:
		
	public slots:
		void set_range(int value);
		
};

#endif // DYNAMIC_SCROLLBAR_H
