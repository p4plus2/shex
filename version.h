#include <QMessageBox>
void display_version_dialog()
{
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v44-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: master\n"
				     "The current commit sha1 is: e5bb10328f1dcaf8aa89cce07c0966916992f332\n"
				     );
	version_info.setWindowFlags(version_info.windowFlags() ^ Qt::WindowCloseButtonHint);
	version_info.exec();
}
