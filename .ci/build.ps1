
$SRC_DIR = $args[0]
$WORK_DIR = $args[1]
$WORK_DIR_SOURCE = "$WORK_DIR/source"
$WORK_DIR_BUILD = "$WORK_DIR/build"
$WORK_DIR_PKG = "$WORK_DIR/package"
$WORK_DIR_DIST = "$WORK_DIR/dist"

# Check input and output directories
if ( !(Test-Path $SRC_DIR -PathType Container) -or !(Test-Path $WORK_DIR -PathType Container)){
	Write-Error "Can not find 'WizNotePlus source folder' or 'Build working folder'"
	exit 1
 }

# Create dirs
cd $WORK_DIR
mkdir -force $WORK_DIR
mkdir -force $WORK_DIR_BUILD
mkdir -force $WORK_DIR_PKG

# Build whole project
conan install $SRC_DIR -if $WORK_DIR_BUILD --build missing -o qtdir=$env:QT_INSTALL_PREFIX
conan build $SRC_DIR -bf $WORK_DIR_BUILD
conan package $SRC_DIR -bf $WORK_DIR_BUILD -pf $WORK_DIR_PKG

cd ..