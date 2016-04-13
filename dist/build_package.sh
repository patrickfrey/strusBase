#!/bin/sh

# DEBIAN
PACKAGEID="strusbase-0.0"

cd pkg/$PACKAGEID
dpkg-buildpackage

