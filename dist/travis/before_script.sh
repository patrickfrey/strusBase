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
		# make sure cmake finds the brew version of gettext
		brew link --force gettext
		sudo ln -s /usr/local/opt/gettext/lib/* /usr/local/lib/.
		sudo ln -s /usr/local/opt/gettext/include/* /usr/local/include/.
		sudo ln -s /usr/local/opt/gettext/bin/* /usr/local/bin/.
		;;
	
	default)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac

