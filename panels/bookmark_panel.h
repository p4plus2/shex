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
#include <QRadioButton>

#include "abstract_panel.h"

class ROM_buffer;

struct bookmark_data{
	int address;
	int size;
	bool code;
	QString description;		
	QColor color;
};

typedef QMap<QString, bookmark_data> bookmark_map;

class bookmark_panel : public QTableView, public abstract_panel
{
		Q_OBJECT
	public:
		explicit bookmark_panel(QWidget *parent = 0);
		virtual QLayout *get_layout();
		virtual void toggle_state(){ state = !state; }
		virtual bool display_state(){ return state; }
		
		void add_bookmark(QString address, bookmark_data bookmark);
		const bookmark_map *map(){ return &bookmark_data_map; }
		
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
		
		bookmark_map bookmark_data_map;

		int row = 0;
		int active_row = 0;
		QStandardItemModel *model = new QStandardItemModel();
		
		QVBoxLayout *box = new QVBoxLayout();	
		QGridLayout *grid = new QGridLayout();
		
		QPushButton *add_button = new QPushButton("Add Bookmark");
		QPushButton *update_button = new QPushButton("Update Bookmark");
		
		QPushButton *reload_button = new QPushButton("Reload Bookmark");
		
		QPushButton *color_button = new QPushButton();
		QLabel *color_label = new QLabel("Color: ");
		
		QRadioButton *code_button = new QRadioButton("Code", this);
		QRadioButton *data_button = new QRadioButton("Data", this);
		                
		QPlainTextEdit *description_input = new QPlainTextEdit();
		QLabel *description_label = new QLabel("Description: ");

		QLineEdit *address_input = new QLineEdit();
		QLabel *address_label = new QLabel("Address: ");
		
		QLineEdit *size_input = new QLineEdit();
		QLabel *size_label = new QLabel("Size: ");
		
		static const int input_padding = 12;
		static bool state;
};

#endif // BOOKMARKS_H
