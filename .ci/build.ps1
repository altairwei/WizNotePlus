
# Check Qt installation
if (-not (Test-Path env:QT_INSTALL_PREFIX)) {
	if (-not (Get-Command qmake -errorAction SilentlyContinue))
	{
		Write-Error "[Error] Qt is required."
		exit 1
	}
}

# Check other tools
if (-not (Get-Command git -errorAction SilentlyContinue))
{
	Write-Error "[Error] git is required."
	exit 1
}

if (-not (Get-Command conan -errorAction SilentlyContinue))
{
	Write-Error "[Error] conan is required."
	exit 1
}

# Setup work folder
$SRC_DIR = $args[0]
$WORK_DIR = $args[1]
$WORK_DIR_SOURCE = "$WORK_DIR/source"
$WORK_DIR_BUILD = "$WORK_DIR/build"
$WORK_DIR_PKG = "$WORK_DIR/package"
$WORK_DIR_DIST = "$WORK_DIR/dist"

# Check input and output directories
if ( !(Test-Path $SRC_DIR -PathType Container) -or !(Test-Path $WORK_DIR -PathType Container)){
	Write-Error "[Error] Can not find 'WizNotePlus source folder' or 'Build working folder'"
	exit 1
}

# Create dirs
cd $WORK_DIR
mkdir -force $WORK_DIR
mkdir -force $WORK_DIR_BUILD
mkdir -force $WORK_DIR_PKG
echo "[Info] WizNotePlus will be built at: $WORK_DIR"

# Copy VC redistributables to release directory for Windows (Github Actions)
if ($args[2] -eq "github actions")
{
	echo "Copy VC redistributables to release directory for Windows (Github Actions)"
	$WORK_DIR_PKG_BIN = "$WORK_DIR/package/bin"
	mkdir -force $WORK_DIR_PKG_BIN
	Copy-Item (vswhere -latest -find 'VC\Redist\MSVC\*\x64\*\msvcp140.dll') $WORK_DIR_PKG_BIN
	Copy-Item (vswhere -latest -find 'VC\Redist\MSVC\*\x64\*\vcruntime140.dll') $WORK_DIR_PKG_BIN
}

# Build whole project
conan install $SRC_DIR -if $WORK_DIR_BUILD --build missing -o qtdir=$env:QT_INSTALL_PREFIX
conan build $SRC_DIR -bf $WORK_DIR_BUILD
conan package $SRC_DIR -bf $WORK_DIR_BUILD -pf $WORK_DIR_PKG

cd ..
