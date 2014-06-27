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

class bookmarks : public QTableView
{
		Q_OBJECT
	public:
		explicit bookmarks(QWidget *parent = 0);
		QGridLayout *get_layout();
		
		void add_bookmark(QString address, QString description);
		
	signals:
		
	public slots:
		void color_clicked();
		void address_updated(QString address);
		void add_clicked();
		void update_clicked();
		void reload_clicked();
		void row_clicked(QModelIndex index);
		
	private:
		void set_color_button(QColor color);
		
		struct bookmark_data{
			int size;
			bool code;
			QString description;		
			QColor color;
		};
		
		QMap<QString, bookmark_data> bookmark_map;
		
		int row = 0;
		int active_row = 0;
		QStandardItemModel *model = new QStandardItemModel();
		
		QGridLayout *grid = new QGridLayout();
		
		QPushButton *add_button = new QPushButton("Add Bookmark");
		QPushButton *update_button = new QPushButton("Update Bookmark");
		
		QPushButton *reload_button = new QPushButton("Reload Bookmark");
		
		QPushButton *color_button = new QPushButton();
		QLabel *color_label = new QLabel("Color: ");
		
		QRadioButton *code = new QRadioButton("Code", this);
		QRadioButton *data = new QRadioButton("Data", this);
		                
		QPlainTextEdit *description_input = new QPlainTextEdit();
		QLabel *description_label = new QLabel("Description: ");

		QLineEdit *address_input = new QLineEdit();
		QLabel *address_label = new QLabel("Address: ");
		
		QLineEdit *size_input = new QLineEdit();
		QLabel *size_label = new QLabel("Size: ");
};

#endif // BOOKMARKS_H
