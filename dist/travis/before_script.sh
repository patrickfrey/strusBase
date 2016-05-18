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
		sudo brew link --force gettext || true
		;;
	
	default)
		echo "ERROR: unknown operating system '$OS'."
		;;
esac

