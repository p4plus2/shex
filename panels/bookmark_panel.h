#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QTableView>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QColormap>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "abstract_panel.h"
#include "panel_manager.h"

class ROM_buffer;

struct bookmark_data{
	enum types{
		CODE = 1,
		PACKED = 2,
		A = 4,
		I = 8,
		BYTE = 16,
		WORD = 32,
		LONG = 64,
		DOUBLE = 128,
		UNKNOWN = 256
	};
		
	int address;
	int size;
	types data_type;
	bool data_is_pointer;
	QString description;		
	QColor color;
};

typedef QMap<QString, bookmark_data> bookmark_map;

class bookmark_panel : public QTableView, public abstract_panel
{
		Q_OBJECT
	public:
		explicit bookmark_panel(panel_manager *parent, hex_editor *editor);
		virtual QLayout *get_layout();
		virtual void toggle_state(){ state = !state; }
		virtual bool display_state(){ return state; }
		
		void add_bookmark(QString address, bookmark_data bookmark);
		
	public slots:
		void color_clicked();
		void address_updated(QString address);
		void context_menu(const QPoint& position);
		void add_clicked();
		void update_clicked();
		void row_clicked(QModelIndex index);
		void row_double_clicked(QModelIndex index);
		void delete_item();
		void read_json();
		void write_json(bool select_name = false);
		void write_as_json();
		
		void create_bookmark(int start, int end, const ROM_buffer *buffer);
		
	private:
		void init_grid_layout();
		void set_color_button(QColor color);
		void layout_adjust();
		int check_address(QString address);
		
		QWidget *input_area = new QWidget(this);
		
		bookmark_map bookmarks;

		int row = 0;
		int active_row = 0;
		QStandardItemModel *model = new QStandardItemModel(this);
		
		QVBoxLayout *box = new QVBoxLayout();
		
		QPushButton *add_button = new QPushButton("Add Bookmark", this);
		QPushButton *update_button = new QPushButton("Update Bookmark", this);
		
		QPushButton *color_button = new QPushButton(this);
		QLabel *color_label = new QLabel("Color: ", this);
		
		QComboBox *data_type = new QComboBox(this);
		QCheckBox *is_pointer = new QCheckBox("Is the data a pointer table", this);
		                
		QPlainTextEdit *description_input = new QPlainTextEdit(this);
		QLabel *description_label = new QLabel("Description: ", this);

		QLineEdit *address_input = new QLineEdit(this);
		QLabel *address_label = new QLabel("Address: ", this);
		
		QLineEdit *size_input = new QLineEdit(this);
		QLabel *size_label = new QLabel("Size: ", this);
		
		bool can_save = false;
		QString file_name = "";
		
		static const int input_padding = 12;
		static bool state;
};

#endif // BOOKMARKS_H
