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
	protected:
		const ROM_buffer *buffer;
		
		virtual void paintEvent(QPaintEvent *event);
		void font_setup();
		
		int get_font_width() const;
		int get_font_height() const;
		int get_rows() const;
		int get_columns() const;
		
		inline int get_offset(){ return editor->get_offset(); }
		inline void set_offset(int o){ editor->set_offset(o); }
		
		inline int get_cursor_nibble(){ return editor->get_cursor_nibble(); }
		inline void set_cursor_nibble(int byte){ editor->set_cursor_nibble(byte); }
		
		inline selection get_selection(){ return editor->get_selection(); }
		inline void get_selection(selection s){ editor->set_selection(s); }
	
		void disable_cursor();

		virtual void mouseMoveEvent(QMouseEvent *event);
		
		virtual QPoint nibble_to_screen(int nibble) = 0;
		virtual int get_line_characters() = 0;
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
		
		hex_editor *editor;
		
		bool display_cursor = true;
		bool cursor_state = true;
		
};

#endif // TEXT_DISPLAY_H
