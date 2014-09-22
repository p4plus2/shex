#ifndef ADDRESS_DISPLAY_H
#define ADDRESS_DISPLAY_H

#include "text_display.h"

class address_display : public text_display
{
		Q_OBJECT
	public:
		using text_display::text_display;
	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual QSize sizeHint () const;
		
		virtual void get_line(int start, int end, QTextStream &stream);
	signals:
		
	public slots:
		
	private:
		const int line_characters = 10;
		
};

#endif // ADDRESS_DISPLAY_H
