#ifndef HEX_EDITOR_H
#define HEX_EDITOR_H

#include <QWidget>
#include <QFile>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QFont>
#include <QClipboard>

class hex_editor : public QWidget
{
	Q_OBJECT
	public:
		explicit hex_editor(QWidget *parent = 0);
		static QString get_address(int address);
		virtual QSize minimumSizeHint() const;

	signals:
		void update_slider(int position);
		void update_range(int value);
		void toggle_scroll_mode(bool mode);

	public slots:
		void update_cursor();
		void slider_update(int position);
		void context_menu(const QPoint& position);
		void cut();
		void copy();
		void paste();
		void delete_text();
		void select_all();
		void disassemble();

	protected:
		virtual void paintEvent(QPaintEvent *event);
		virtual void keyPressEvent(QKeyEvent *event);
		virtual void wheelEvent(QWheelEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void mouseMoveEvent(QMouseEvent *event);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void resizeEvent(QResizeEvent *event);

	private:
		QFile ROM;
		QByteArray buffer;
		int columns;
		int rows;
		int offset;
		QPoint cursor_position;
		QPoint selection_start;
		QPoint selection_current;
		bool cursor_state;
		bool is_dragging;
		bool selection_active;
		QFont font;
		int font_width;
		int font_height;
		int total_width;
		int vertical_offset;
		int vertical_shift;
		QClipboard *clipboard;
		bool scroll_mode;
		
		void font_setup();
		QString get_line(int index);
		void update_nibble(char byte);
		void update_cursor_position(int x, int y);
		int get_buffer_position(int x, int y, bool byte_align = true);
		
		inline int column_width(int size){ return size * font_width; }
		inline int column_height(int size){ return size * font_height; }
		inline int get_max_lines(){ return buffer.size() / columns - rows; }
		inline bool check_paste_data(){ return !clipboard->mimeData()->hasText(); }
};

#endif // HEX_EDITOR_H
