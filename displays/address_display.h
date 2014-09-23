#ifndef ADDRESS_DISPLAY_H
#define ADDRESS_DISPLAY_H

#include "text_display.h"

class address_display : public text_display
{
		Q_OBJECT
	public:
		address_display(const ROM_buffer *b, QWidget *parent);
	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual QSize sizeHint () const;
		
		virtual int get_line_characters(){ return line_characters; }
		virtual void get_line(int start, int end, QTextStream &stream);
	signals:
		
	public slots:
		
	private:
		const int line_characters = 9;
		
};

#endif // ADDRESS_DISPLAY_H
