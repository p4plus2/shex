#ifndef TEXT_DISPLAY_H
#define TEXT_DISPLAY_H

#include <QWidget>
#include <QFont>
#include <QCache>
#include <QStaticText>

#include "rom_buffer.h"
#include "hex_editor.h"
#include "panels/bookmark_panel.h"
#include "settings_manager.h"
#include "editor_font.h"

class QPainter;
class QTextStream;

class text_display : public QWidget
{
		Q_OBJECT
	public:
		explicit text_display(const ROM_buffer *b, hex_editor *parent = 0);
		void update_display();
		void set_auto_scroll_speed(int speed);
		inline void invalidate_cache(){ row_cache.clear(); }
		inline void disable_cursor_timer() { killTimer(cursor_timer_id); }
		static int get_rows(){ return rows; }
		static int get_columns(){ return columns; }
		static int get_rows_by_columns(){ return rows * columns; }
		virtual QSize minimumSizeHint() const { return sizeHint(); }
	protected:
		const ROM_buffer *buffer;

		QPoint clip_mouse(int x, int y);
		QPoint clip_screen(QPoint position){ return clip_mouse(position.x(), position.y()); }
		
		inline int get_offset(){ return editor->get_offset(); }
		inline void set_offset(int o){ editor->set_offset(o); }
		
		inline int get_cursor_nibble(){ return editor->get_cursor_nibble(); }
		inline void set_cursor_nibble(int byte){ editor->set_cursor_nibble(byte); }
		
		inline selection get_selection(){ return editor->get_selection(); }
		inline void set_selection(selection s){ editor->set_selection(s); }
		
		virtual void paintEvent(QPaintEvent *event);
		virtual void paint_selection(QPainter &painter, selection &selection_area, const QColor &color);
		virtual bool event(QEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);
		virtual void timerEvent(QTimerEvent *event);
		
		virtual int screen_to_nibble(QPoint position, bool byte_align = false) = 0;
		virtual QPoint nibble_to_screen(int nibble) = 0;
		virtual int get_line_characters() const = 0;
		virtual void get_line(int start, int end, QTextStream &stream) = 0;
	signals:
		void character_typed(unsigned char key, bool update_byte);
	
	public slots:
		void update_size();
		
	private:				
		QCache<int, QStaticText> row_cache;
		bookmark_map *bookmarks = nullptr;
		
		hex_editor *editor;
		bool cursor_state = true;
		int cursor_timer_id = 0;
		
		int scroll_timer_id = 0;
		int scroll_direction = 1;
		int scroll_speed = 1;
		static const int scroll_timer_speed = 20;
		
		static const int cursor_width = 1;
		static QColor selection_color;
		
		static int rows;
		static int columns;
};

#endif // TEXT_DISPLAY_H
