#include <QMessageBox>
void display_version_dialog()
{
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v20").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: master\n"
				     "The current commit sha1 is: 4813cda181beead48355ae83187940441965b763\n"
				     );
	version_info.setWindowFlags(version_info.windowFlags() ^ Qt::WindowCloseButtonHint);
	version_info.exec();
}
