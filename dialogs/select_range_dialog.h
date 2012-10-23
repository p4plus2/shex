#ifndef select_range_DIALOG_H
#define select_range_DIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

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
		QLabel *start_label;
		QLabel *end_label;
		QLineEdit *start_input;
		QLineEdit *end_input;
		QRadioButton *absolute;
		QRadioButton *relative;
		
		QPushButton *select_range;
		QPushButton *close;
		
		int parse_input(QString input);
};

#endif // select_range_DIALOG_H
