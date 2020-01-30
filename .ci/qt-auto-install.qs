var QT_VERSION_MAJOR = installer.environmentVariable("QT_VERSION_MAJOR");
var QT_VERSION_MINOR = installer.environmentVariable("QT_VERSION_MINOR");
var QT_VERSION_PATCH = installer.environmentVariable("QT_VERSION_PATCH");
var qt_id = "qt." + "qt" + QT_VERSION_MAJOR + "." + QT_VERSION_MAJOR + QT_VERSION_MINOR + QT_VERSION_PATCH

var INSTALL_COMPONENTS = [
    qt_id + ".gcc_64",
    qt_id + ".qtwebengine",
    qt_id + ".qtwebengine.gcc_64",
];

function Controller() {
    console.log("Control Script: Start to install Qt library automaticlly.");
    installer.autoRejectMessageBoxes();

    installer.installationFinished.connect(function() {
        gui.clickButton(buttons.NextButton);
    })

    installer.currentPageChanged.connect(function(page) {
        var wgt = gui.pageById(page);
        console.log("Control Script: Current '" + wgt.objectName + "' changed")
    })
}

Controller.prototype.DynamicTelemetryPluginFormCallback = function() {
    console.log("Control Script: Enter 'Dynamic Telemetry Plugin Form'")
    var widget = gui.currentPageWidget();
    widget.TelemetryPluginForm.statisticGroupBox.disableStatisticRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.WelcomePageCallback = function() {
    // click delay here because the next button is initially disabled for ~1 second
    console.log("Control Script: Enter 'Welcome Page'")
    gui.clickButton(buttons.NextButton, 3000);
}

Controller.prototype.CredentialsPageCallback = function() {
    console.log("Control Script: Enter 'Credentials Page'");
    // If the network is opened, you are required to login.
    var wgt = gui.currentPageWidget();
    wgt.loginWidget.EmailLineEdit.setText("wiznoteplus@mywiz.cn");
    wgt.loginWidget.PasswordLineEdit.setText("xEt9EQwX");    
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ManualLicensePageCallback = function () {
    console.log("Control Script: Enter 'Manual License Page'");
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ObligationsPageCallback = function() {
    console.log("Control Script: Enter 'Obligations Page'")
    var wgt = gui.currentPageWidget();
    // obligationsAgreement is a button not a checkbox!
    wgt.obligationsAgreement.click();
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
    console.log("Control Script: Enter 'Introduction Page'")
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.TargetDirectoryPageCallback = function()
{
    console.log("Control Script: Enter 'Target Directory Page'");
    var qtdir = installer.environmentVariable("QT_INSTALL_TARGET_DIR");
    if (!qtdir) {
        console.log("Control Script: QT_INSTALL_TARGET_DIR not set");
        qtdir = installer.value("HomeDir") + "/Qt";
    }
    console.log("Control Script: Qt will be installed to " + qtdir);
    gui.currentPageWidget().TargetDirectoryLineEdit.setText(qtdir);
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    console.log("Control Script: Enter 'Component Selection Page'")
    var widget = gui.currentPageWidget();
    widget.deselectAll();
    INSTALL_COMPONENTS.forEach(function(comp_name) {
        widget.selectComponent(comp_name);
    })
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function() {
    console.log("Control Script: Enter 'License Agreement Page'")
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    console.log("Control Script: Enter 'Start Menu Directory Page'")
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    console.log("Control Script: Enter 'Ready For Installation Page'")
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.PerformInstallationPageCallback = function() {
    console.log("Control Script: Enter 'Perform Installation Page'")
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function() {
    console.log("Control Script: Enter 'Finished Page'")
    var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm;
    if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
        checkBoxForm.launchQtCreatorCheckBox.checked = false;
    }
    gui.clickButton(buttons.FinishButton);
}

function showObjectKeys(obj) {
    var obj_keys = Object.keys(obj);
    for (i in obj_keys) {
        console.log("- " + obj_keys[i] + ": " + obj[obj_keys[i]])
    }
}