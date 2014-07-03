#ifndef HEX_DISPLAY_H
#define HEX_DISPLAY_H

#include "text_display.h"
#include <QSize>

class hex_display : public text_display
{
		Q_OBJECT
	public:
		using text_display::text_display;
	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual QSize sizeHint () const;
		
	signals:
		
	public slots:
		
};

#endif // HEX_DISPLAY_H
