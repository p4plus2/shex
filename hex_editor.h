#ifndef HEX_EDITOR_H
#define HEX_EDITOR_H

#include "selection.h"
#include "rom_buffer.h"

#include <QWidget>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QFont>
#include <QTimer>

class address_display;
class ascii_display;
class hex_display;

class hex_editor : public QWidget
{
	Q_OBJECT
	public:
		explicit hex_editor(QWidget *parent, QString file_name, QUndoGroup *undo_group, bool new_file = false);
		~hex_editor();
		inline ROM_buffer *get_buffer(){ return buffer; }
		inline int get_relative_position(int address){ return get_buffer_position(cursor_position) + address; }
		void set_focus();
		void update_window();
		inline void save(QString path) { buffer->save(path); update_save_state(-save_state); }
		inline bool can_save(){ return save_state; }
		inline bool new_file(){ return is_new; }
		inline QString load_error() { return ROM_error; }
		QString get_file_name() { return buffer->get_file_name(); }
		
		//refact inlines
		inline int get_offset(){ return offset; }
		inline void set_offset(int o){ offset = o; }
		
		inline int get_cursor_nibble(){ return cursor_nibble; }
		inline void set_cursor_nibble(int byte){ cursor_nibble = byte; update_window(); }
		
		inline selection get_selection(){ return selection_bytes; }
		inline void set_selection(selection s){ selection_bytes = s; update_window(); }

	signals:
		void update_slider(int position);
		void update_range(int value);
		void update_status_text(QString text);
		void toggle_scroll_mode(bool mode);
		void selection_toggled(bool state);
		void focused(bool has_focus);
		void clipboard_usable(bool usable);
		void can_save(bool save);
		void send_disassemble_data(int start, int end, const ROM_buffer *buffer);
		void send_bookmark_data(int start, int end, const ROM_buffer *buffer);

	public slots:
		void handle_typed_character(unsigned char key, bool update_byte = false);
		void update_undo_action(bool direction);
		void goto_offset(int address);
		void select_range(int start, int end);
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
		void branch();
		void jump();
		void disassemble();
		void create_bookmark();
		void count(QString find, bool mode);
		void search(QString find, bool direction, bool mode);
		void replace(QString find, QString replace, bool direction, bool mode);
		void replace_all(QString find, QString replace, bool mode);
		
		inline void clipboard_changed(){ emit clipboard_usable(buffer->check_paste_data()); }

	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual void paint_selection(QPainter &painter);
		virtual void keyPressEvent(QKeyEvent *event);
		virtual void wheelEvent(QWheelEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);

	private:
		address_display *address;
		hex_display *hex;
		ascii_display *ascii;
		
		//refactor adds
		int cursor_nibble = 0;
		selection selection_bytes;
		//end refactor adds
		
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
		bool is_active = true;
		int font_width; //needs removed
		int font_height;
		int vertical_offset = 6;
		int vertical_shift;

		int byte_column_width;
		int total_byte_column_width;
		bool scroll_mode = false;
		bool auto_scrolling;
		QTimer *scroll_timer = new QTimer(this);
		int hex_offset;
		int ascii_offset;
		int scroll_speed;
		bool scroll_direction;
		QPoint mouse_position;
		int save_state = 0;
		bool is_new;
		QString ROM_error = "";
		
		QString get_status_text();
		int get_selection_point(QPoint point);
		bool get_selection_range(int &start, int &end);
		bool follow_selection(bool type);
		int get_buffer_position(int x, int y, bool byte_align = true);
		int get_buffer_position(QPoint &point, bool byte_align = true);
		QPoint get_byte_position(int address, bool byte_align = true);
		void move_cursor_nibble(int delta);
		void update_nibble(char byte);
		void update_cursor_position(int x, int y, bool do_update = true);
		void update_selection_position(int amount);
		void update_selection(int x, int y);
		void search_error(int error, QString find = "", QString replace_with = "");
		
		inline int column_width(int size) const { return size * font_width; }
		inline int column_height(int size) const { return size * font_height; }
		inline int to_ascii_column(int x){ return column_width(14+columns*3+(x-font_width*11)/font_width/3); }
		inline int to_hex_column(int x){ return column_width(11+(x-font_width*(14+columns*3))/font_width*3); }
		inline int get_max_lines(){ return buffer->size() / columns - rows; }
		inline void set_selection_active(bool s){ selection_active = s; emit selection_toggled(s); }
		inline void update_save_state(int direction){ save_state += direction; emit can_save(!save_state); }
		
		static const QString offset_header;
};
#endif // HEX_EDITOR_H
