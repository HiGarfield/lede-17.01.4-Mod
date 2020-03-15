#/bin/bash

{
	echo "Input commit message:" &&
	read commit_msg &&
	./clean_all.sh &&
	./generate_workflow_yml.sh &&
	./new_version.sh &&
	git add . &&
	git commit -m "$commit_msg"
} || exit 1
