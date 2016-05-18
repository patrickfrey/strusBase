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
		# || true below to cope rc=1 when boost is already installed.
		brew install \
			boost \
			cmake \
			gettext \
			|| true
		# sadly our intl cmake macro cannot find the correct gettext
		# version without this (brew --link never worked correctly for us):
		sudo ln -fs /usr/local/opt/gettext/lib/* /usr/local/lib/.
		sudo ln -fs /usr/local/opt/gettext/include/* /usr/local/include/.
		sudo ln -fs /usr/local/opt/gettext/bin/* /usr/local/bin/.
		;;
	
	default)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac

