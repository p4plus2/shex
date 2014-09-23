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
		virtual void mousePressEvent(QMouseEvent *event);
		virtual QSize sizeHint () const;
		
		virtual int get_line_characters(){ return line_characters; }
		virtual void get_line(int start, int end, QTextStream &stream);
		QPoint map_to_byte(int x, int y, bool byte_align = false);
	signals:
		
	public slots:
		
	private:
		const int line_characters = 47; //don't count trailing space
		
};

#endif // HEX_DISPLAY_H
