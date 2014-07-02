#ifndef ADDRESS_DISPLAY_H
#define ADDRESS_DISPLAY_H

#include "text_display.h"

class address_display : public text_display
{
		Q_OBJECT
	public:
		explicit address_display(QWidget *parent = 0);
	protected:
		virtual void paintEvent(QPaintEvent *event);
		
	signals:
		
	public slots:
		
};

#endif // ADDRESS_DISPLAY_H
