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
	version_info.setText(QString("The current version is: v220-WIP").leftJustified(100));
	version_info.setWindowTitle("Version info");
	version_info.setDetailedText(
        "The current build branch is: notify_on_filechanged\n"
        "The current commit sha1 is: 5993cc26630ec8d1171c07c2a7b0b7eaa7ed49c4\n"
        "Compiled with: Microsoft (R) C/C++ Optimizing Compiler Version 19.35.32217.1 for x64\n" + OS
	);
	version_info.setWindowFlags(version_info.windowFlags());
	version_info.exec();
}
