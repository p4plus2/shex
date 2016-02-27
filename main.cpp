#include "main_window.h"
#include "debug.h"

void message_handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	Q_UNUSED(context);
	QString text;
	switch (type){
		case QtDebugMsg:
			text = QString("Debug: %1").arg(message);
		break;
		case QtWarningMsg:
			text = QString("Warning: %1").arg(message);
		break;
		case QtCriticalMsg:
			text = QString("Critical: %1").arg(message);
		break;
		case QtFatalMsg:
			text = QString("Fatal: %1").arg(message);
		break;
		default:
			text = QString("Unknown message type: %1").arg(message);
		break;
	}
	QFile log("debug.log");
	log.open(QIODevice::WriteOnly);
	QTextStream log_stream(&log);
	log_stream << text << endl;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
#ifdef LOG_TO_FILE
	qInstallMessageHandler(message_handler);
#endif
	
	QCoreApplication::setOrganizationName("p4programing");
	QCoreApplication::setApplicationName("shex");
	     
	main_window window;
	window.show();
	
	return a.exec();
}
