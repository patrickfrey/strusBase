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
			brew install gcc48 --enable-all-languages
			brew link --force gcc48
		fi
		brew install \
			cmake \
			boost \
			gettext
		brew link --force gettext
		;;
	
	default)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac

