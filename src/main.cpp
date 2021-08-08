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

//-------------------------------------------------------------------
// 程序初始化安装
//-------------------------------------------------------------------

const char* g_lpszDesktopFileName = "\
[Desktop Entry]\n\
Exec=%1WizNote\n\
Icon=wiznote\n\
Type=Application\n\
Terminal=false\n\
Name=%2\n\
GenericName=%3\n\
Categories=WizNote;\n\
Name[en_US]=WizNote\n\
GenericName[en_US.UTF-8]=WizNote\n\
";


#ifdef Q_OS_LINUX

/**
 * @brief Linux特定的安装过程
 * 
 */
void installOnLinux()
{
    // 准备.desktop文件内容
    QString appPath = Utils::WizPathResolve::appPath();
    QString strText = WizFormatString3(g_lpszDesktopFileName,
                                       appPath,
                                       QObject::tr("WizNote"),
                                       QObject::tr("WizNote"));
    //
    QString applicationsPath = QDir::homePath() + "/.local/share/applications/";
    ::WizEnsurePathExists(applicationsPath);
    //
    QString iconsBasePath = QDir::homePath() + "/.local/share/icons/hicolor/";
    ::WizEnsurePathExists(iconsBasePath);
    // 导出不同尺寸图标文件
    CWizStdStringArray arrayIconSize;
    arrayIconSize.push_back("16");
    arrayIconSize.push_back("32");
    arrayIconSize.push_back("48");
    arrayIconSize.push_back("64");
    arrayIconSize.push_back("128");
    arrayIconSize.push_back("256");
    for (CWizStdStringArray::const_iterator it = arrayIconSize.begin();
        it != arrayIconSize.end();
        it++)
    {
        QString iconSize = *it;
        QString iconPathName = iconSize + "x" + iconSize;
        QString iconFullPath = iconsBasePath + iconPathName + "/apps/";
        WizEnsurePathExists(iconFullPath);
        // 将内部logo图片导出到文件夹
        QString resourceName = ":/logo_" + iconSize + ".png";
        QPixmap pixmap(resourceName);
        if (pixmap.isNull())
            continue;
        //
        pixmap.save(iconFullPath + "wiznote.png");
    }
    // 创建桌面文件
    QString desktopFileName = applicationsPath + "wiznote.desktop";
    ::WizSaveUnicodeTextToUtf8File(desktopFileName, strText, false);
    //
    chmod(desktopFileName.toUtf8(), ACCESSPERMS);
}
#endif

//-------------------------------------------------------------------
// 登录与启动
//-------------------------------------------------------------------

int mainCore(int argc, char *argv[])
{
    // 设置高分屏支持
    //-------------------------------------------------------------------

#ifdef Q_OS_LINUX
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef Q_OS_MAC
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef Q_OS_WIN
    // 暂时先采用UI缩放+字体缩小的方案来适配Windows高分屏
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setAttribute(Qt::AA_Use96Dpi);
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

    QtWebEngine::initialize();
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);

#if QT_VERSION >= 0x051000
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
#endif

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    // 配置QtApp和Debug
    //-------------------------------------------------------------------

#ifdef Q_OS_WIN
    QFont appFont = WizCreateWindowsUIFont(app);
    QApplication::setFont(appFont);
#endif // Q_OS_WIN

    // Debug 输出
    qInstallMessageHandler(Utils::WizLogger::messageHandler); // 输出到 Wiznote 消息控制台
    //qInstallMessageHandler(nullptr); // 输出到 Qt Debug console
    // 设置高分屏图标
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // 设置应用名和组织名用于QSetting
    QApplication::setApplicationName(QObject::tr("WizNote"));
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

    QString stylefile, style;
    stylefile = WizGetSkinResourcePath(userSettings.skin()) + "style.qss";
    if (WizLoadUnicodeTextFromFile(stylefile, style)) {
        app.setStyleSheet(style);
    }

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

#ifdef Q_OS_LINUX
    // 判断Linux端是否已经安装过Wiznode
    if (globalSettings->value("Common/Installed", 0).toInt() == 0)
    {
        globalSettings->setValue("Common/Installed", 1);
        installOnLinux();
    }
#endif

    // 登录程序
    //-------------------------------------------------------------------

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

