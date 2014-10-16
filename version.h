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
	version_info.setText(QString("The current version is: v117-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: master\n"
				     "The current commit sha1 is: 3d523ce125157eed089d4ebb964fca29cf124149\n"
				     "Compiled with: gcc (Gentoo 4.8.3 p1.1, pie-0.5.9) 4.8.3\n" + OS
				     );
	version_info.setWindowFlags(version_info.windowFlags());
	version_info.exec();
}
