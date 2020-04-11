#/bin/bash

{
	echo "Input commit message:" &&
	read commit_msg &&
	./clean_all.sh &&
	git add . &&
	git commit -m "$commit_msg"
} || exit 1
