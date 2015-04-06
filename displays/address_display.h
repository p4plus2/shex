#ifndef ADDRESS_DISPLAY_H
#define ADDRESS_DISPLAY_H

#include "text_display.h"

class address_display : public text_display
{
		Q_OBJECT
	public:
		address_display(const ROM_buffer *b, hex_editor *parent);
	protected:
		virtual void paintEvent(QPaintEvent *event);
		
		virtual int get_line_characters() const { return line_characters; }
		virtual void get_line(int start, int end, QTextStream &stream);
		
		virtual int screen_to_nibble(QPoint position, bool byte_align = false);
		virtual QPoint nibble_to_screen(int nibble);
	signals:
		
	public slots:
		
	private:
		const int line_characters = 9;
		
};

#endif // ADDRESS_DISPLAY_H
