#!/bin/bash
# Script to bundle Qt app and dependencies for distribution
# Usage: ./package-qt-app.sh

set -e

APP=QTqt
BUILD_DIR=cmake-build-debug
DIST_DIR=dist
QT_PLUGINS_DIR=$(dirname $(ldd $BUILD_DIR/$APP | grep libQt6Core | awk '{print $3}'))/../plugins

# Clean and create dist dir
rm -rf $DIST_DIR
mkdir -p $DIST_DIR/platforms

# Copy executable
cp $BUILD_DIR/$APP $DIST_DIR/

# Copy Qt libraries
ldd $BUILD_DIR/$APP | grep "/libQt" | awk '{print $3}' | xargs -I '{}' cp -v '{}' $DIST_DIR/

# Copy platform plugin (libqxcb)
cp -v $QT_PLUGINS_DIR/platforms/libqxcb.so $DIST_DIR/platforms/

# Copy assets if needed
cp -r asset $DIST_DIR/

# Copy resources if needed
if [ -f $BUILD_DIR/qrc_resources.cpp ]; then
  cp $BUILD_DIR/qrc_resources.cpp $DIST_DIR/
fi

# Print run instructions
echo "\nBundle created in $DIST_DIR. To run on another system, use:"
echo "  export QT_QPA_PLATFORM_PLUGIN_PATH=\"\$(pwd)/$DIST_DIR/platforms\""
echo "  cd $DIST_DIR && ./QTqt"

