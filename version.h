#include <QMessageBox>
void display_version_dialog()
{
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v41-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: master\n"
				     "The current commit sha1 is: 901b7d9ebf62aba4c802c5055bfbcb1afa4f16d6\n"
				     );
	version_info.setWindowFlags(version_info.windowFlags() ^ Qt::WindowCloseButtonHint);
	version_info.exec();
}
