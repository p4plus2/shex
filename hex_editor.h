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
		inline int get_relative_position(int address){ return cursor_nibble / 2 + address; }
		void set_focus();
		void update_window();
		inline void save(QString path) { buffer->save(path); update_save_state(-save_state); }
		inline bool can_save(){ return save_state; }
		inline bool new_file(){ return is_new; }
		inline QString load_error() { return ROM_error; }
		QString get_file_name() { return buffer->get_file_name(); }
		
		//refact adds
		inline int get_offset(){ return offset; }
		void set_offset(int o);
		
		inline int get_cursor_nibble(){ return cursor_nibble; }
		inline void set_cursor_nibble(int byte){ cursor_nibble = byte; update_window(); }
		
		inline selection get_selection(){ return selection_area; }
		inline void set_selection(selection s){ selection_area = s; update_window(); }

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
		virtual void keyPressEvent(QKeyEvent *event);
		virtual void wheelEvent(QWheelEvent *event);

	private:
		address_display *address;
		hex_display *hex;
		ascii_display *ascii;
		
		//refactor adds
		int cursor_nibble = 0;
		selection selection_area;
		//end refactor adds
		
		ROM_buffer *buffer;
		int columns = 16;
		int rows = 32;
		int offset = 0;
		bool click_side = false;
		bool is_active = true;
		int font_width; //needs removed
		int font_height;

		bool scroll_mode = false;
		bool auto_scrolling;
		QTimer *scroll_timer = new QTimer(this);
		int scroll_speed;
		bool scroll_direction;
		int save_state = 0;
		bool is_new;
		QString ROM_error = "";
		
		QString get_status_text();
		bool follow_selection(bool type);
		void move_cursor_nibble(int delta);
		void update_nibble(char byte);
		void search_error(int error, QString find = "", QString replace_with = "");
	
		inline int get_max_lines(){ return buffer->size() / columns - rows; }
		inline void update_save_state(int direction){ save_state += direction; emit can_save(!save_state); }
		
		static const QString offset_header;
};
#endif // HEX_EDITOR_H
