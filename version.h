#include <QMessageBox>
#include <QSysInfo>

void display_version_dialog()
{
	QString OS;
	#if defined(Q_OS_LINUX)
		OS = "Operating system: Linux";
	#elif defined(Q_OS_WIN32)
		OS = "Operating system: Windows";
	#elif defined(Q_OS_MAC)
		OS = "Operating system: Mac";
	#else
		OS = "Operating system: Unknown or unsupported";
	#endif
	QMessageBox version_info;
	version_info.setText(QString("The current version is: v212-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: disassembly_refactor\n"
				     "The current commit sha1 is: 8122648140284515c3f4c317de172eb29061eda3\n"
				     "Compiled with: gcc (Gentoo 4.9.3 p1.0, pie-0.6.2) 4.9.3\n" + OS
				     );
	version_info.setWindowFlags(version_info.windowFlags());
	version_info.exec();
}
