#ifndef HEX_DISPLAY_H
#define HEX_DISPLAY_H

#include "text_display.h"

class hex_display : public text_display
{
		Q_OBJECT
	public:
		using text_display::text_display;
	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual void keyPressEvent(QKeyEvent *event);
		virtual QSize sizeHint () const;
		
		virtual int get_line_characters() const { return line_characters; }
		virtual void get_line(int start, int end, QTextStream &stream);
		virtual int screen_to_nibble(QPoint position, bool byte_align = false);
		virtual QPoint nibble_to_screen(int nibble);
	signals:
		
	public slots:
		
	private:
		const int line_characters = 47; //don't count trailing space
		
};

#endif // HEX_DISPLAY_H
