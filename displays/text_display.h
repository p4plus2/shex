#ifndef TEXT_DISPLAY_H
#define TEXT_DISPLAY_H

#include <QWidget>
#include <QFont>
#include "rom_buffer.h"

class QPainter;

class text_display : public QWidget
{
		Q_OBJECT
	public:
		explicit text_display(const ROM_buffer *b, QWidget *parent = 0);
	protected:
		const ROM_buffer *buffer;
		
		void font_setup();
		void set_painter_font(QPainter &painter);
		
		virtual void mouseMoveEvent(QMouseEvent *event);
	signals:
		
	public slots:
		
	private:		
		//make these static at some point, no need to duplicate them
		int font_width;
		int font_height;
		QFont font;
		
};

#endif // TEXT_DISPLAY_H
