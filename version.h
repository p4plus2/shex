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
	version_info.setText(QString("The current version is: v213-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
				     "The current build branch is: disassembly_refactor\n"
				     "The current commit sha1 is: b60306b1a5b7fdca39560882ed5242735e10ed1b\n"
				     "Compiled with: gcc (Gentoo 5.3.0 p1.0, pie-0.6.5) 5.3.0\n" + OS
				     );
	version_info.setWindowFlags(version_info.windowFlags());
	version_info.exec();
}
