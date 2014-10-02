#ifndef TEXT_DISPLAY_H
#define TEXT_DISPLAY_H

#include <QWidget>
#include <QFont>
#include "rom_buffer.h"
#include "hex_editor.h"

class QPainter;
class QTextStream;

class text_display : public QWidget
{
		Q_OBJECT
	public:
		explicit text_display(const ROM_buffer *b, hex_editor *parent = 0);
		void update_display();
		void set_auto_scroll_speed(int speed);
		static int get_rows(){ return rows; }
		static int get_columns(){ return columns; }
		static QFont get_font(){ return font; }
		
	protected:
		const ROM_buffer *buffer;
		QFont font_setup();
		
		static int get_font_width(){ return font_width; }
		static int get_font_height(){ return font_height; }

		QPoint clip_mouse(int x, int y);
		QPoint clip_screen(QPoint position){ return clip_mouse(position.x(), position.y()); }
		
		inline int get_offset(){ return editor->get_offset(); }
		inline void set_offset(int o){ editor->set_offset(o); }
		
		inline int get_cursor_nibble(){ return editor->get_cursor_nibble(); }
		inline void set_cursor_nibble(int byte){ editor->set_cursor_nibble(byte); }
		
		inline selection get_selection(){ return editor->get_selection(); }
		inline void set_selection(selection s){ editor->set_selection(s); }
		
		virtual void paintEvent(QPaintEvent *event);
		virtual void paint_selection(QPainter &painter, selection &selection_area);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);
		virtual void timerEvent(QTimerEvent *event);
		
		virtual int screen_to_nibble(QPoint position, bool byte_align = false) = 0;
		virtual QPoint nibble_to_screen(int nibble) = 0;
		virtual int get_line_characters() = 0;
		virtual void get_line(int start, int end, QTextStream &stream) = 0;
	signals:
		void character_typed(unsigned char key, bool update_byte);
		
	private:		
		//make these static at some point, no need to duplicate them
		static int font_width;
		static int font_height;
		static QFont font;
		
		//also make these static at some point
		static int rows;
		static int columns;
		
		hex_editor *editor;
		
		bool cursor_state = true;
		int scroll_timer_id = 0;
		int cursor_timer_id = 0;
		int scroll_direction = 1;
		int scroll_speed = 1;
		static const int scroll_timer_speed = 20;
};

#endif // TEXT_DISPLAY_H
