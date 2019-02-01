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
#include "share/WizDatabaseManager.h"
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
#include "WizDocumentWebEngine.h"
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

/** 
 * @brief 初始化内核
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 */
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
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif

    // 初始化主进程和QtWebEngine
    //-------------------------------------------------------------------

#ifdef Q_OS_LINUX
    // create single application for linux
    WizSingleApplication app(argc, argv, "Special-Message-for-WizNote-SingleApplication");
    if (app.isRunning())
    {
        app.sendMessage(WIZ_SINGLE_APPLICATION);
        return 0;
    }
    // 初始化Chrome内核
    QtWebEngine::initialize();
#else
    // 创建Win和Mac端 Qt主进程
    QApplication app(argc, argv);
    //
#ifdef BUILD4APPSTORE
    QDir dir(QApplication::applicationDirPath()); //进入exe文件所在的绝对路径
    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath())); //设置库路径
#endif
    // 初始化Win和Mac端Chrome内核
    QtWebEngine::initialize();

#ifdef BUILD4APPSTORE
    WizIAPHelper helper;
    helper.validteReceiptOnLauch();
#endif
#endif

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);

    // 配置QtApp和Debug
    //-------------------------------------------------------------------

#ifdef Q_OS_WIN
    QFont appFont = WizCreateWindowsUIFont(app, WizGetWindowsFontName());
    appFont.setPixelSize(14); // Windows 端自动缩放UI后缩小字体大小
    //appFont.setPointSize(12);
    QApplication::setFont(appFont);
#endif
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

    // tooltip 样式
    app.setStyle(QStyleFactory::create("fusion"));
    app.setStyleSheet("QToolTip { \
                    font: 12px; \
                    color:#000000; \
                    padding:0px 1px; \
                    background-color: #F8F8F8; \
                    border:0px;}");

    // 初始化用户设置
    //-------------------------------------------------------------------

    // setup settings
    QSettings::setDefaultFormat(QSettings::IniFormat);

    /// 全局设置
    /** 设置文件在～/.wiznote/wiznote.ini或者%HOMEPATH%/Wiznote/wiznote.ini里面. */
    QSettings* globalSettings = new QSettings(Utils::WizPathResolve::globalSettingsFile(), QSettings::IniFormat);
    WizGlobal::setGlobalSettings(globalSettings);
    //

    // use 3 times(30M) of Qt default usage
    int nCacheSize = globalSettings->value("Common/Cache", 10240*3).toInt();
    QPixmapCache::setCacheLimit(nCacheSize);

    QString strUserGuid = globalSettings->value("Users/DefaultUserGuid").toString();
    QList<WizLocalUser> localUsers;
    WizGetLocalUsers(localUsers);    
    QString strAccountFolderName = WizGetLocalFolderName(localUsers, strUserGuid);

    QString strPassword;
    WizUserSettings userSettings(strAccountFolderName);

    /// 获取用户设置
    QSettings* settings = new QSettings(Utils::WizPathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
    WizGlobal::setSettings(settings);

    /// 外置编辑器设置
    /*
    QSettings* extEditorSettings = new QSettings(
                Utils::WizPathResolve::dataStorePath() + strAccountFolderName + "/externalEditor.ini", QSettings::IniFormat);
    extEditorSettings->beginGroup("Editor_0");
    extEditorSettings->setValue("Name", "Typora");
    extEditorSettings->setValue("ProgramFile", "/usr/bin/typora");
    extEditorSettings->setValue("Arguments", "%1"); // %1 将被传入文件地址
    extEditorSettings->setValue("TextEditor", "1");
    extEditorSettings->setValue("UTF8Encoding", "0");
    extEditorSettings->endGroup();
    */

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


    bool bAutoLogin = userSettings.autoLogin();
    strPassword = userSettings.password(); // 竟然用base64加密解密 (╬￣皿￣)

    if (bAutoLogin && !strPassword.isEmpty()) {
        // 当勾选自动登录且密码不为空时，自动登录
        bFallback = false;
    }    

    //
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

    //
    QString strUserId = WizGetLocalUserId(localUsers, strUserGuid);
    bool isNewRegisterAccount = false;
    // manually login 手动登录
    if (bFallback)
    {
        // 创建登录窗口，并启动事件循环
        WizLoginDialog loginDialog(strLocale, localUsers);
        if (QDialog::Accepted != loginDialog.exec())
            return 0;

        // 如果默认UserId为空，或者登录ID不等于默认UserId
        if (strUserId.isEmpty() || loginDialog.loginUserGuid() != strUserGuid)
        {   // 重新获取用户文件夹
            strAccountFolderName = WizGetLocalFolderName(localUsers, loginDialog.loginUserGuid());
            if (strAccountFolderName.isEmpty())
            {
                strAccountFolderName = loginDialog.userId();
            }
            qDebug() << "login user id : " << loginDialog.userId();
            // 写入新用户设置
            settings = new QSettings(Utils::WizPathResolve::userSettingsFile(strAccountFolderName), QSettings::IniFormat);
            WizGlobal::setSettings(settings);
        }
        // 获取用户输入账号密码
        strPassword = loginDialog.password();
        strUserId = loginDialog.userId();
        isNewRegisterAccount = loginDialog.isNewRegisterAccount();
    }
    else
    { // WizBox企业登录设置
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

    //
    //
    // reset locale for current user. 将设置重置到当前用户
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
    // 设置用户语言
    WizCommonApiEntry::setLanguage(strLocale);

    // 登录数据库管理
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
    // 绑定消息通知
    QObject::connect(&app, SIGNAL(messageAvailable(QString)), &window,
                     SLOT(on_application_messageAvailable(QString)));
#endif

    //settings->setValue("Users/DefaultUser", strUserId);

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
        // 清理密码？
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

    //WizQueuedThreadsShutdown();
    // clean up 清理临时文件
    QString strTempPath = Utils::WizPathResolve::tempPath();
    ::WizDeleteAllFilesInFolder(strTempPath);

    return ret;
}

