#ifndef GOTO_DIALOG_H
#define GOTO_DIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

class goto_dialog : public QDialog
{
		Q_OBJECT
	public:
		explicit goto_dialog(QWidget *parent = 0);
		
	signals:
		void triggered(int address, bool mode);
		
	public slots:
		void address_entered();
		
	private:
		QLabel *label;
		QLineEdit *offset_input;
		QRadioButton *absolute;
		QRadioButton *relative;
		
		QPushButton *goto_offset;
		QPushButton *close;
};

#endif // GOTO_DIALOG_H
