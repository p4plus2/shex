#ifndef ASCII_DISPLAY_H
#define ASCII_DISPLAY_H

#include "text_display.h"
#include <QSize>

class ascii_display : public text_display
{
		Q_OBJECT
	public:
		using text_display::text_display;
	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual QSize sizeHint () const;
		
	signals:
		
	public slots:
		
	private:
		const int line_characters = 16;
		
};

#endif // ASCII_DISPLAY_H
