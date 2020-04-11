#/bin/bash

{
	./generate_workflow_yml.sh &&
	./new_version.sh &&
	git add . &&
	git commit -m "refresh version and config files"
	git push origin master &&
	./make_tar.sh
} || exit 1

