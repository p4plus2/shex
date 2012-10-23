#ifndef select_range_DIALOG_H
#define select_range_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>

class select_range_dialog : public QDialog
{
		Q_OBJECT
	public:
		explicit select_range_dialog(QWidget *parent = 0);
		
	signals:
		void triggered(int start, int end, bool mode);
		
	public slots:
		void range_entered();
		
	private:
		QLabel *start_label = new QLabel("&Starting SNES offset: ", this);
		QLineEdit *start_input = new QLineEdit(this);
		QLabel *end_label = new QLabel("&Ending SNES offset: ", this);
		QLineEdit *end_input = new QLineEdit(this);
		QRadioButton *absolute = new QRadioButton("Absolute offsets", this);
		QRadioButton *relative = new QRadioButton("Relative offsets", this);
		
		QPushButton *select_range = new QPushButton("Select range", this);
		QPushButton *close = new QPushButton("Close", this);
		
		int parse_input(QString input);
};

#endif // select_range_DIALOG_H
