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
#include <QMap>
#include <QRadioButton>

class hex_editor;
class ROM_buffer;

struct bookmark_data{
	int address;
	int size;
	bool code;
	QString description;		
	QColor color;
};

typedef QMap<QString, bookmark_data> bookmark_map;

class bookmarks : public QTableView
{
		Q_OBJECT
	public:
		explicit bookmarks(hex_editor *parent = 0);
		QVBoxLayout *get_layout();
		
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
		void toggle_display(bool state);
		
	private:
		void init_grid_layout();
		void set_color_button(QColor color);
		void layout_adjust();
		int check_address(QString address);
		
		hex_editor *editor;
		QWidget *input_area = new QWidget(this);
		
		bookmark_map bookmark_data_map;
		
		static bool display;
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
};

#endif // BOOKMARKS_H
