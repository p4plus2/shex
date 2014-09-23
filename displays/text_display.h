#ifndef TEXT_DISPLAY_H
#define TEXT_DISPLAY_H

#include <QWidget>
#include <QFont>
#include "rom_buffer.h"

class QPainter;
class QTextStream;

class text_display : public QWidget
{
		Q_OBJECT
	public:
		explicit text_display(const ROM_buffer *b, QWidget *parent = 0);
	protected:
		const ROM_buffer *buffer;
		
		virtual void paintEvent(QPaintEvent *event);
		void font_setup();
		
		int get_font_width() const;
		int get_font_height() const;
		int get_rows() const;
		int get_columns() const;
		
		void set_cursor_position(int x, int y);

		virtual void mouseMoveEvent(QMouseEvent *event);
		
		virtual void get_line(int start, int end, QTextStream &stream) = 0;
	signals:
		
	public slots:
		void update_cursor_state();
		
	private:		
		//make these static at some point, no need to duplicate them
		int font_width;
		int font_height;
		
		//also make these static at some point
		int rows = 32;
		int columns = 16;
		
		//this too
		QFont font;
		
		bool display_cursor = true;
		bool cursor_state = true;
		QPoint cursor_position = {0,0};
		
};

#endif // TEXT_DISPLAY_H
