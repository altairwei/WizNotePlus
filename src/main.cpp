#include <QtGlobal>
#include <QApplication>
#include <QTreeWidget>
#include <QMessageBox>
#include <QIcon>
#include <QDir>
#include <QPixmapCache>
#include <QTranslator>
#include <QProcess>
#include <QSettings>
#include <QDesktopServices>
#include <QSslConfiguration>
#include <QNetworkProxy>
#include <QtWebEngine>
#include <QStyleFactory>
#include <QWebEngineSettings>
#include <QVersionNumber>

#include <sys/stat.h>

#include "utils/WizPathResolve.h"
#include "utils/WizLogger.h"
#include "utils/WizStyleHelper.h"
#include "share/WizSettings.h"
#include "share/WizWin32Helper.h"
#include "database/WizDatabaseManager.h"
#include "share/WizSingleApplication.h"
#include "share/WizThreads.h"
#include "share/WizGlobal.h"

#include "core/WizNoteManager.h"

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#include "mac/WizIAPHelper.h"
#endif

#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "sync/WizAvatarHost.h"
#include "WizThumbCache.h"
#include "WizMainWindow.h"
#include "WizLoginDialog.h"

#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/wiznote"
#endif

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifdef Q_OS_LINUX
#include <gnu/libc-version.h>
#endif

void disableWebengineSandboxIfNeeded()
{
#ifndef Q_OS_LINUX
    return;
#else
#   if (QT_VERSION > QT_VERSION_CHECK(6, 0, 0))
    return;
#   else
    const char* version = gnu_get_libc_version();
    QVersionNumber current = QVersionNumber::fromString(version);
    QVersionNumber compareTo(2, 34);
    if (current >= compareTo) {
        qDebug() << "Disabling Qt WebEngine sandbox as GLIBC"
                 << version << "will break it";
        qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    }
#   endif
#endif
}

//-------------------------------------------------------------------
// 登录与启动
//-------------------------------------------------------------------

int mainCore(int argc, char *argv[])
{
    // 设置高分屏支持
    //-------------------------------------------------------------------

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    // Init Application and QtWebEngine
    //-------------------------------------------------------------------

#ifdef Q_OS_LINUX
    // create single application for linux
    WizSingleApplication app(argc, argv, "Special-Message-for-WizNote-SingleApplication");
    if (app.isRunning())
    {
        app.sendMessage(WIZ_SINGLE_APPLICATION);
        return 0;
    }
#else

    // create application for Windows and MacOS
    QApplication app(argc, argv);

#ifdef BUILD4APPSTORE
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif // BUILD4APPSTORE

#ifdef BUILD4APPSTORE
    WizIAPHelper helper;
    helper.validteReceiptOnLauch();
#endif // BUILD4APPSTORE

#endif // Q_OS_LINUX

    disableWebengineSandboxIfNeeded();
    QtWebEngine::initialize();
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
#endif

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    // 配置QtApp和Debug
    //-------------------------------------------------------------------

    // Debug 输出
    qInstallMessageHandler(Utils::WizLogger::messageHandler); // 输出到 Wiznote 消息控制台

    // 设置应用名和组织名用于QSetting
    QApplication::setApplicationName(QObject::tr("WizNotePlus"));
    QApplication::setOrganizationName(QObject::tr("cn.wiz.wiznoteformac"));

    QIcon icon;
    icon.addPixmap(QPixmap(":/logo_16.png"));
    icon.addPixmap(QPixmap(":/logo_32.png"));
    icon.addPixmap(QPixmap(":/logo_48.png"));
    icon.addPixmap(QPixmap(":/logo_64.png"));
    icon.addPixmap(QPixmap(":/logo_128.png"));
    icon.addPixmap(QPixmap(":/logo_256.png"));
    QApplication::setWindowIcon(icon);

#ifdef Q_OS_MAC
    wizMacInitUncaughtExceptionHandler();
    wizMacRegisterSystemService();

    // init sys local for crash report
    QString sysLocal = QLocale::system().name();
    QTranslator translatorSys;
    QString sysLocalFile = Utils::WizPathResolve::localeFileName(sysLocal);
    translatorSys.load(sysLocalFile);
    app.installTranslator(&translatorSys);

    initCrashReporter();

    app.removeTranslator(&translatorSys);

    WizQueuedThreadsInit();

    //FIXME: 在Mac osx安全更新之后存在ssl握手问题，此处进行特殊处理
    QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(conf);
#endif

    // 获取默认用户设置
    //-------------------------------------------------------------------

    QSettings::setDefaultFormat(QSettings::IniFormat);

    /** Global INI setting. File is located at ~/.wiznote/wiznote.ini or %HOMEPATH%/Wiznote/wiznote.ini */
    QSettings* globalSettings = new QSettings(Utils::WizPathResolve::globalSettingsFile(), QSettings::IniFormat);
    WizGlobal::setGlobalSettings(globalSettings);

    // use 3 times(30M) of Qt default usage
    int nCacheSize = globalSettings->value("Common/Cache", 10240*3).toInt();
    QPixmapCache::setCacheLimit(nCacheSize);

    /** GUID of default user. */
    QString strUserGuid = globalSettings->value("Users/DefaultUserGuid").toString();
    QList<WizLocalUser> localUsers;
    WizGetLocalUsers(localUsers);
    /** Folder name of default user. */
    QString strAccountFolderName = WizGetLocalFolderName(localUsers, strUserGuid);

    /** Setting from META table within user sqlite database. */
    WizUserSettings userSettings(strAccountFolderName);

    /** User specific INI setting. */
    QSettings* settings = new QSettings(Utils::WizPathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
    WizGlobal::setSettings(settings);

    // 样式
    app.setStyle(QStyleFactory::create("fusion"));
    app.setStyleSheet(WizLoadSkinStyleSheet(userSettings.skin()));

    // 语言本地化
    //-------------------------------------------------------------------

    // setup locale for welcome dialog
    QString strLocale = userSettings.locale();
    QLocale::setDefault(strLocale);

    QTranslator translatorWizNote;
    QString strLocaleFile = Utils::WizPathResolve::localeFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    app.installTranslator(&translatorWizNote);

    QTranslator translatorQt;
    strLocaleFile = Utils::WizPathResolve::qtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    app.installTranslator(&translatorQt);

    // 登录程序
    //-------------------------------------------------------------------

    QFont font(WizUserSettings::kDefaultUIFontFamily, WizUserSettings::kDefaultUIFontSize);
    app.setFont(font);

    // figure out auto login or manually login
    bool bFallback = true;

    // FIXME: move to WizService initialize
    WizToken token;

    QString strPassword = userSettings.password();

    bool bAutoLogin = userSettings.autoLogin();
    if (bAutoLogin && !strPassword.isEmpty()) {
        bFallback = false;
    }

    //set network proxy
    WizSettings wizSettings(Utils::WizPathResolve::globalSettingsFile());
    if (wizSettings.getProxyStatus())
    {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(wizSettings.getProxyHost());
        proxy.setPort(wizSettings.getProxyPort());
        proxy.setUser(wizSettings.getProxyUserName());
        proxy.setPassword(wizSettings.getProxyPassword());
        QNetworkProxy::setApplicationProxy(proxy);
    }

    // Login procedure

    QString strUserId = WizGetLocalUserId(localUsers, strUserGuid);
    bool isNewRegisterAccount = false;

    if (bFallback)
    {
        WizLoginDialog loginDialog(strLocale, localUsers);
        if (QDialog::Accepted != loginDialog.exec())
            return 0;

        if (strUserId.isEmpty() || loginDialog.loginUserGuid() != strUserGuid)
        {
            // 获取另一个用户的账户文件夹
            strAccountFolderName = WizGetLocalFolderName(localUsers, loginDialog.loginUserGuid());
            if (strAccountFolderName.isEmpty())
            {
                strAccountFolderName = loginDialog.userId();
            }
            qDebug() << "login user id : " << loginDialog.userId();

            // 写入另一个用户设置
            settings = new QSettings(Utils::WizPathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
            WizGlobal::setSettings(settings);
        }

        strPassword = loginDialog.password();
        strUserId = loginDialog.userId();
        isNewRegisterAccount = loginDialog.isNewRegisterAccount();
    }
    else
    {
        // WizBox企业登录设置
        if (userSettings.serverType() == EnterpriseServer)
        {
            WizCommonApiEntry::setEnterpriseServerIP(userSettings.enterpriseServerIP());
        }
        else if (userSettings.serverType() == WizServer ||
                 (userSettings.serverType() == NoServer && !userSettings.myWizMail().isEmpty()))
        {
            WizCommonApiEntry::setEnterpriseServerIP(WIZNOTE_API_SERVER);
        }
    }

    // Reset locale for current user.
    userSettings.setAccountFolderName(strAccountFolderName);
    userSettings.setUserId(strUserId);
    strLocale = userSettings.locale();

    app.removeTranslator(&translatorWizNote);
    strLocaleFile = Utils::WizPathResolve::localeFileName(strLocale);
    translatorWizNote.load(strLocaleFile);
    app.installTranslator(&translatorWizNote);

    app.removeTranslator(&translatorQt);
    strLocaleFile = Utils::WizPathResolve::qtLocaleFileName(strLocale);
    translatorQt.load(strLocaleFile);
    app.installTranslator(&translatorQt);

    WizCommonApiEntry::setLanguage(strLocale);

    QFont uiFont(userSettings.UIFontFamily(), userSettings.UIFontSize());
    app.setFont(uiFont);

    // 登录数据库管理器
    WizDatabaseManager dbMgr(strAccountFolderName);
    if (!dbMgr.openAll()) {
        QMessageBox::critical(NULL, "", QObject::tr("Can not open database"));
        return 0;
    }

    qDebug() << "set user id for token ; " << strUserId;
    WizToken::setUserId(strUserId);
    WizToken::setPasswd(strPassword);

    dbMgr.db().setPassword(::WizEncryptPassword(strPassword));
    dbMgr.db().updateInvalidData();

    // FIXME: move to plugins
    WizAvatarHost avatarHost;

    // FIXME: move to core plugin initialize
    WizThumbCache cache;

    // 启动Wiz主窗口
    WizMainWindow window(dbMgr);

#ifdef Q_OS_LINUX
    QObject::connect(&app, SIGNAL(messageAvailable(QString)), &window,
                     SLOT(on_application_messageAvailable(QString)));
#endif

    window.show();
    window.init();

#ifdef Q_OS_MAC
    //start and set safari extension
    WIZUSERINFO userInfo;
    dbMgr.db().getUserInfo(userInfo);
    WizExecuteOnThread(WIZ_THREAD_DEFAULT, [strUserId, userInfo](){
        updateShareExtensionAccount(strUserId, userInfo.strUserGUID, userInfo.strMywizEmail ,userInfo.strDisplayName);
        //readShareExtensionAccount();
    });
#endif

    //create introduction note for new register users
    WizNoteManager noteManager(dbMgr);
    noteManager.updateTemplateJS(userSettings.locale());
    noteManager.downloadTemplatePurchaseRecord();
    if (isNewRegisterAccount)
    {
        noteManager.createIntroductionNoteForNewRegisterAccount();
    }

    int ret = app.exec();
    if (window.isLogout()) {
        userSettings.setPassword("");
#ifndef BUILD4APPSTORE
        QProcess::startDetached(argv[0], QStringList());
#else
        QString strAppFile = QApplication::applicationDirPath().remove("/Contents/MacOS");
        if (!QProcess::startDetached("/usr/bin/open -W "+strAppFile))
        {
            QMessageBox::information(0, "Info", "open " + strAppFile + " failed");
        }
#endif
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = mainCore(argc, argv);

    // clean up the last temporary files
    QString strTempPath = Utils::WizPathResolve::tempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    return ret;
}

