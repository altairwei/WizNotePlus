#!/bin/bash
# Xcode Run Script Phase
# Script that builds the doxygen documentation for the project and loads the docset into Xcode.
#
# Use the following to adjust the value of the $DOXYGEN_PATH User-Defined Setting:
#   Binary install location: /Applications/Doxygen.app/Contents/Resources/doxygen
#   Source build install location: /usr/local/bin/doxygen
#
# If the config file doesn't exist, run 'doxygen -g $SOURCE_ROOT/Doxyfile.config' to
# a get default file.
#
DOXYGEN_PATH="/Applications/Doxygen.app/Contents/Resources/doxygen"
DOXYFILE_NAME="Doxyfile.config"
INPUT_PATH="${SOURCE_ROOT}/../src/"
OUTPUT_PATH="${SOURCE_ROOT}/../"
DOXYFILE_PATH="${SOURCE_ROOT}/../${DOXYFILE_NAME}"
DOCSET_BUNDLE_ID="com.google.${PRODUCT_NAME}"

# Check if Doxyfile exists.
if ! [ -f $DOXYFILE_PATH ]
then
    echo "Doxyfile config does not exist"
    # Generate default Doxyfile
    $DOXYGEN_PATH -g $DOXYFILE_PATH
fi

# Create temporary Doxyfile copy.
cp $DOXYFILE_PATH $TEMP_DIR/$DOXYFILE_NAME

# Append the proper input/output directories and docset info to the temp config file.
# This works even though values are assigned higher up in the file. Easier than sed.
echo -e "INPUT\t\t\t\t   = ${INPUT_PATH}\nOUTPUT_DIRECTORY\t   = ${OUTPUT_PATH}\nGENERATE_DOCSET\t\t   = YES\nDOCSET_BUNDLE_ID\t   = ${DOCSET_BUNDLE_ID}" >> $TEMP_DIR/$DOXYFILE_NAME

# Run doxygen on the updated config file.
# Note: doxygen creates a Makefile that does most of the heavy lifting.
$DOXYGEN_PATH $TEMP_DIR/$DOXYFILE_NAME

# Clean up Doxyfile copy.
rm -f $TEMP_DIR/$DOXYFILE_NAME

# make will invoke docsetutil. Take a look at the Makefile to see how this is done.
make -C $OUTPUT_PATH/html install

# Applescript to load the docset into Xcode for immediate availability.
echo -e "tell application \"Xcode\"\n\tload documentation set with path \"/Users/${USER}/Library/Developer/Shared/Documentation/DocSets/${DOCSET_BUNDLE_ID}.docset\"\nend tell" > $TEMP_DIR/loadDocSet.scpt

# Run the load-docset applescript command.
osascript $TEMP_DIR/loadDocSet.scpt

# Clean up load-docset applescript file.
rm -f $TEMP_DIR/loadDocSet.scpt

echo "Success."
exit 0