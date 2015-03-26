#ifndef ASCII_DISPLAY_H
#define ASCII_DISPLAY_H

#include "text_display.h"

class ascii_display : public text_display
{
		Q_OBJECT
	public:
		using text_display::text_display;
	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual void keyPressEvent(QKeyEvent *event);
		virtual QSize sizeHint () const;
		
		virtual int get_line_characters(){ return line_characters; }
		virtual void get_line(int start, int end, QTextStream &stream);
		virtual int screen_to_nibble(QPoint position, bool byte_align = false);
		virtual QPoint nibble_to_screen(int nibble);
	signals:
		
	public slots:
		
	private:
		const int line_characters = 16;
		
};

#endif // ASCII_DISPLAY_H
