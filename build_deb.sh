#!/bin/bash
cp ResizeQt debian/usr/bin/
dpkg-deb --build debian/
mv debian.deb ResizeQt.deb
