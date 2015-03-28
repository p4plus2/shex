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

#include "abstract_panel.h"
#include "panel_manager.h"

class ROM_buffer;

enum bookmark_data_types{
	CODE_A8_I8,
	CODE_A8_I16,
	CODE_A16_I8,
	CODE_A16_I16,
	DATA_PACKED_BYTE,
	DATA_PACKED_WORD,
	DATA_PACKED_LONG,
	DATA_PACKED_DOUBLE,
	DATA_BYTE,
	DATA_WORD,
	DATA_LONG,
	DATA_DOUBLE
};

struct bookmark_data{
	int address;
	int size;
	bookmark_data_types data_type;
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
		void add_clicked();
		void update_clicked();
		void reload_clicked();
		void row_clicked(QModelIndex index);
		
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
		
		QPushButton *reload_button = new QPushButton("Reload Bookmark", this);
		
		QPushButton *color_button = new QPushButton(this);
		QLabel *color_label = new QLabel("Color: ", this);
		
		QComboBox *data_type = new QComboBox(this);
		                
		QPlainTextEdit *description_input = new QPlainTextEdit(this);
		QLabel *description_label = new QLabel("Description: ", this);

		QLineEdit *address_input = new QLineEdit(this);
		QLabel *address_label = new QLabel("Address: ", this);
		
		QLineEdit *size_input = new QLineEdit(this);
		QLabel *size_label = new QLabel("Size: ", this);
		
		static const int input_padding = 12;
		static bool state;
};

#endif // BOOKMARKS_H
