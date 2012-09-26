#!/bin/sh
last_commit=$(git rev-parse HEAD)
commit_count=$(git log --pretty=format:'' | wc -l)
version_number=$((commit_count + 1))
git_branch=$(git branch 2>/dev/null| sed -n '/^\*/s/^\* //p')

if (($(git status 2> /dev/null | wc -l) == 8)); then
	if git status 2> /dev/null | grep version\.h > /dev/null; then
		wip_build=""
	else
		wip_build="-WIP"
	fi
fi


rm -f version.cpp

echo "#include <QMessageBox>
void display_version_dialog()
{
	QMessageBox version_info;
	version_info.setText(QString(\"The current version is: v${version_number}${wip_build}\").leftJustified(100));
	version_info.setWindowTitle(\"Version info\");
	version_info.setDetailedText(
				     \"The current build branch is: ${git_branch}\\n\"
				     \"The current commit sha1 is: ${last_commit}\\n\"
				     );
	version_info.setWindowFlags(version_info.windowFlags() ^ Qt::WindowCloseButtonHint);
	version_info.exec();
}" > version.h
