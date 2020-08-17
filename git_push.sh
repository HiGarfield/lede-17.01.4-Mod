#/bin/bash

{
	./generate_workflow_yml.sh &&
	./new_version.sh &&
	ver_num=$(cat version)
	git add . &&
	git commit -m "$ver_num" &&
	git push --all origin --force &&
	./make_tar.sh
} || exit 1

