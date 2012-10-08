#include <QMessageBox>
void display_version_dialog()
{
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v32-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: master\n"
				     "The current commit sha1 is: b5a655fb91c784855fc8a54ddbb8cdbc57e6f37f\n"
				     );
	version_info.setWindowFlags(version_info.windowFlags() ^ Qt::WindowCloseButtonHint);
	version_info.exec();
}
