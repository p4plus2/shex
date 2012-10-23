#ifndef HEX_EDITOR_H
#define HEX_EDITOR_H

#include <QWidget>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QFont>
#include <QTimer>

#include "rom_buffer.h"

class hex_editor : public QWidget
{
	Q_OBJECT
	public:
		explicit hex_editor(QWidget *parent = 0, QString file_name = "", QUndoGroup *undo_group = 0);
		~hex_editor();
		virtual QSize minimumSizeHint() const;
		QString get_file_name();
		void set_focus();

	signals:
		void update_slider(int position);
		void update_range(int value);
		void update_status_text(QString text);
		void toggle_scroll_mode(bool mode);

	public slots:
		void update_cursor_state();
		void update_undo_action();
		void goto_offset(int address, bool mode);
		void select_range(int start, int end, bool mode);
		void slider_update(int position);
		void scroll_mode_changed();
		void auto_scroll_update();
		void control_auto_scroll(bool enabled);
		void context_menu(const QPoint& position);
		void cut();
		void copy();
		void paste(bool raw = false);
		void delete_text();
		void select_all();
		void disassemble();

	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual void paint_selection(QPainter &painter);
		virtual bool event(QEvent *e);
		virtual void keyPressEvent(QKeyEvent *event);
		virtual void handle_typed_character(unsigned char key, bool update_byte = false);
		virtual void wheelEvent(QWheelEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);

	private:
		ROM_buffer *buffer;
		int columns = 16;
		int rows = 32;
		int offset = 0;
		QPoint cursor_position;
		QPoint selection_start;
		QPoint selection_current;
		bool cursor_state = false;
		bool is_dragging = false;
		bool selection_active = false;
		bool click_side = false;
		QFont font;
		int font_width;
		int font_height;
		int vertical_offset = 6;
		int vertical_shift;
		bool scroll_mode = false;
		bool auto_scrolling;
		QTimer *scroll_timer = new QTimer(this);
		int scroll_speed;
		bool scroll_direction;
		QPoint mouse_position;
		
		void font_setup();
		QString get_status_text();
		QPoint get_selection_point(QPoint point);
		bool get_selection_range(int position[2]);
		int get_buffer_position(int x, int y, bool byte_align = true);
		int get_buffer_position(QPoint &point, bool byte_align = true);
		QPoint get_byte_position(int address, bool byte_align = true);
		void update_nibble(char byte);
		void update_cursor_position(int x, int y, bool do_update = true);
		void update_selection_position(int amount);
		void update_selection(int x, int y);
		void update_window();
		
		inline int column_width(int size){ return size * font_width; }
		inline int column_height(int size){ return size * font_height; }
		inline int to_ascii_column(int x){ return column_width(14+columns*3+(x-font_width*11)/font_width/3); }
		inline int to_hex_column(int x){ return column_width(11+(x-font_width*(14+columns*3))/font_width*3); }
		inline int get_max_lines(){ return buffer->size() / columns - rows; }
		
		static const QString offset_header;
};

#endif // HEX_EDITOR_H
