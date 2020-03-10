#/bin/bash

{
	git push origin master &&
	./make_tar.sh
} || exit 1

