#include <QMessageBox>
void display_version_dialog()
{
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v31-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: master\n"
				     "The current commit sha1 is: 7abb8cc25a924de81195e8cc7768798de1d31c0d\n"
				     );
	version_info.setWindowFlags(version_info.windowFlags() ^ Qt::WindowCloseButtonHint);
	version_info.exec();
}
