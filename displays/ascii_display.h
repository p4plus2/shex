#ifndef ASCII_DISPLAY_H
#define ASCII_DISPLAY_H

#include "text_display.h"

class ascii_display : public text_display
{
		Q_OBJECT
	public:
		explicit ascii_display(QWidget *parent = 0);
	protected:
		virtual void paintEvent(QPaintEvent *event);
		
	signals:
		
	public slots:
		
};

#endif // ASCII_DISPLAY_H
