#ifndef HEX_DISPLAY_H
#define HEX_DISPLAY_H

#include "text_display.h"

class hex_display : public text_display
{
		Q_OBJECT
	public:
		explicit hex_display(QWidget *parent = 0);
	protected:
		virtual void paintEvent(QPaintEvent *event);
		
	signals:
		
	public slots:
		
};

#endif // HEX_DISPLAY_H
