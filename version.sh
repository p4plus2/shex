last_commit=$(git rev-parse HEAD)
version_number=$(git shortlog | grep -E '^[ ]+\w+' | wc -l)
git_branch=$(git branch 2>/dev/null| sed -n '/^\*/s/^\* //p')
if git diff --quiet 2>/dev/null >&2; then
    wip_build=""
else
    wip_build="-WIP"
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
