#!/bin/sh

OS=$(uname -s)

case $OS in
	Linux)
		sudo apt-get update -qq
		sudo apt-get install -y \
			cmake \
			libboost-all-dev
		;;
		
	Darwin)
		brew update
		if test "X$CC" = "Xgcc"; then
			brew install gcc48 --enable-all-languages || true
			brew link --force gcc48 || true
		fi
		brew install \
			cmake \
			boost \
			gettext \
			|| true
		# make sure cmake finds the brew version of gettext
		brew link --force gettext || true
		;;
	
	*)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac

