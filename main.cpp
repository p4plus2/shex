#include <QtGui/QApplication>
#include <QCleanlooksStyle>
#include "main_window.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	main_window window;
	window.show();
	
	return a.exec();
}
