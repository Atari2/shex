if ((Get-Command cl -ErrorAction SilentlyContinue) -eq $null) {
    Write-Error "cl.exe not found, please run this script from a Developer Command Prompt for Visual Studio"
    exit
}

$last_commit=(git rev-parse HEAD)
$commit_count=(git log --pretty=format:'' | Measure-Object).Count
$version_number=($commit_count + 1)
$git_branch=(-split (git branch | Select-String -Pattern "^\*"))[1]
$compiler_version=((& cl /Bv 2>&1) -split '\n')[0]

$statusoutput = (git status 2> /dev/null | Measure-Object).Count
$versionmodified = (git status 2> /dev/null | Select-String version.h)

if ($statusoutput -eq 8) {
	if ($versionmodified) {
		$wip_build=""
    } else {
		$wip_build="-WIP"
    }
} elseif ($statusoutput -ne 2) {
	$wip_build="-WIP"
} else {
	$wip_build=""
}

Write-Output "#include <QMessageBox>
#include <QSysInfo>

void display_version_dialog()
{
	QString OS;
	#if defined(Q_OS_LINUX)
		OS = `"Operating system: Linux`";
	#elif defined(Q_OS_WIN32)
		OS = `"Operating system: Windows`";
	#elif defined(Q_OS_MAC)
		OS = `"Operating system: Mac`";
	#else
		OS = `"Operating system: Unknown or unsupported`";
	#endif
	QMessageBox version_info;
	version_info.setText(QString(`"The current version is: v$version_number$wip_build`").leftJustified(100));
	version_info.setWindowTitle(`"Version info`");
	version_info.setDetailedText(
        `"The current build branch is: $git_branch\n`"
        `"The current commit sha1 is: $last_commit\n`"
        `"Compiled with: $compiler_version\n`" + OS
	);
	version_info.setWindowFlags(version_info.windowFlags());
	version_info.exec();
}" > version.h
