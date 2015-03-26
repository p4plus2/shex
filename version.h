#include <QMessageBox>
#include <QSysInfo>

void display_version_dialog()
{
	QString OS;
	#if defined(Q_OS_LINUX)
		OS = "Operating system: Linux";
	#elif defined(Q_OS_WIN32)
		OS = "Operating system: Window";
	#elif defined(Q_OS_MAC)
		OS = "Operating system: Mac";
	#else
		OS = "Operating system: Unknown or unsupported";
	#endif
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v130-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: menu_refactor2\n"
				     "The current commit sha1 is: bbdb2c04c5c5682119ef4777f242ed40ce77ebe4\n"
				     "Compiled with: gcc (Gentoo 4.8.4 p1.0, pie-0.6.1) 4.8.4\n" + OS
				     );
	version_info.setWindowFlags(version_info.windowFlags());
	version_info.exec();
}
