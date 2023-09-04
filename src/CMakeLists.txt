add_subdirectory(libs)

#============================================================================
# Sources
#============================================================================

set(wiznote_SOURCES_MAC
    mac/WizMacHelper.mm
    mac/WizUserInfoWidgetBaseMac.mm
    mac/WizUserInfoWidgetBaseMac_mm.cpp
    widgets/WizUserInfoWidgetBase.cpp
    utils/WizNotify.mm
    mac/WizIAPHelper.mm
    mac/rmstore/RMAppReceipt.mm
    mac/rmstore/RMStoreAppReceiptVerificator.mm
    mac/WizNotificationCenter.mm
    widgets/WizIAPDialog.cpp
    mac/DTWebArchive.m
)

set(wiznote_HEADERS_MAC
    mac/WizMacHelper.h
    mac/WizMacHelper_mm.h
    mac/WizUserInfoWidgetBaseMac_mm.h
    widgets/WizUserInfoWidgetBase.h
    mac/WizIAPHelper.h
    mac/rmstore/RMAppReceipt.h
    mac/rmstore/RMStoreAppReceiptVerificator.h
    mac/WizNotificationCenter.h
    widgets/WizIAPDialog.h
    mac/DTWebArchive.h
)


set(wiznote_FORMS_MAC
    ui/WizIAPDialog.ui
)

set(wiznote_SOURCES
    utils/WizLogger.cpp
    utils/WizMisc.cpp
    utils/WizPinyin.cpp
    utils/WizNotify.cpp

    share/WizZip.cpp
    share/WizQtHelper.cpp
    share/WizMisc.cpp
    share/WizObject.cpp
    share/WizKMCore.cpp
    share/WizGlobal.cpp

    database/WizDatabase.cpp
    database/WizIndex.cpp
    database/WizIndexBase.cpp
    database/WizThumbIndex.cpp
    database/sqlite3.c
    database/cppsqlite3.cpp
    database/WizDatabaseManager.cpp
    database/WizOnlineDatabase.cpp

    share/WizSettings.cpp
    share/WizUI.cpp
    share/WizMd5.cpp
    share/WizHtml2Zip.cpp
    share/WizWin32Helper.cpp
    share/WizDrawTextHelper.cpp
    share/WizUIHelper.cpp
    share/WizCommonUI.cpp
    share/WizAnimateAction.cpp
    share/WizPopupWidget.cpp
    share/WizMultiLineListWidget.cpp
    share/WizFileIconProvider.cpp
    share/WizWindowsHelper.cpp
    share/WizEnc.cpp
    share/WizZiwReader.cpp
    html/WizHtmlReader.cpp
    html/WizHtmlCollector.cpp
    html/WizHtmlTool.cpp
    html/WizGumboHelper.cpp
    share/WizSearch.cpp
    widgets/WizScrollBar.cpp
    sync/WizToken.cpp
    sync/WizApiEntry.cpp
    share/WizObjectDataDownloader.cpp
    sync/WizAvatarHost.cpp
    sync/WizKMServer.cpp
    sync/WizKMSync.cpp
    sync/WizAsyncApi.cpp
    sync/WizAvatarUploader.cpp
    sync/WizJSONServerBase.cpp
    sync/WizSync.cpp
    widgets/WizUserInfoWidget.cpp
    widgets/WizUserInfoWidgetBase.cpp
    widgets/WizSegmentedButton.cpp
    widgets/WizExternalEditorSettingDialog.cpp
    utils/WizStyleHelper.cpp
    utils/WizPathResolve.cpp
    share/WizWindowTitleBar.cpp
    share/WizShadowEffect.cpp
    widgets/WizTagBar.cpp
    widgets/WizTrayIcon.cpp
    share/WizEventLoop.cpp
    share/WizObjectOperator.cpp
    widgets/WizAboutDialog.cpp
    widgets/WizLocalProgressWebView.cpp
    widgets/WizCrashReportDialog.cpp
    widgets/WizCustomToolBar.cpp
    widgets/WizTipsWidget.cpp
    widgets/WizShareLinkDialog.cpp
    share/WizAnalyzer.cpp
    share/WizTranslater.cpp
    widgets/WizAdvancedSearchDialog.cpp
    share/WizUdpClient.cpp
    share/WizMessageBox.cpp
    share/WizFileMonitor.cpp
    share/WizLockedFile.cpp
    share/WizLocalPeer.cpp
    share/WizSingleApplication.cpp
    widgets/WizImageButton.cpp
    widgets/WizCodeEditorDialog.cpp
    widgets/WizFramelessWebDialog.cpp
    widgets/WizScreenShotWidget.cpp
    widgets/WizVerificationCodeDialog.cpp
    widgets/WizEmailShareDialog.cpp
    widgets/WizTemplatePurchaseDialog.cpp
    widgets/WizExecutingActionDialog.cpp
    share/WizWebEngineView.cpp
    widgets/WizTableSelector.cpp
    widgets/WizUserServiceExprDialog.cpp

    share/jsoncpp/jsoncpp.cpp
    share/WizRequest.cpp

    jsplugin/JSPlugin.cpp
    jsplugin/JSPluginManager.cpp
    jsplugin/JSPluginSpec.cpp
    jsplugin/JSPluginWidgets.cpp
    jsplugin/JSRepl.cpp

    api/PublicAPIsServer.cpp
    api/WebSocketClientWrapper.cpp
    api/WebSocketTransport.cpp

    gui/tabbrowser/WizMainTabBrowser.cpp
    gui/tabbrowser/AbstractTabPage.cpp
    gui/tabbrowser/FullScreenWindow.cpp
    gui/tabbrowser/FullScreenNotification.cpp
    gui/tabbrowser/WebEngineWindow.cpp
    gui/tabbrowser/WizWebsiteView.cpp
    gui/tabbrowser/TabButton.cpp

    gui/categoryviewer/WizCategoryView.cpp
    gui/categoryviewer/WizCategoryViewItemBase.cpp
    gui/categoryviewer/WizCategoryViewSectionItem.cpp
    gui/categoryviewer/WizCategoryViewMessageItem.cpp
    gui/categoryviewer/WizCategoryViewShortcutItem.cpp
    gui/categoryviewer/WizCategoryViewSearchItem.cpp
    gui/categoryviewer/WizCategoryViewFolderItem.cpp
    gui/categoryviewer/WizCategoryViewTagItem.cpp
    gui/categoryviewer/WizCategoryViewGroupItem.cpp
    gui/categoryviewer/WizCategoryViewItem.cpp
    gui/categoryviewer/WizFolderView.cpp
    gui/categoryviewer/WizFolderSelector.cpp

    WizDevToolsDialog.cpp

    gui/doclistviewer/WizDocumentListView.cpp
    gui/doclistviewer/WizDocumentListViewBaseItem.cpp
    gui/doclistviewer/WizDocumentListViewDocumentItem.cpp
    gui/doclistviewer/WizDocumentListViewSectionItem.cpp
    gui/doclistviewer/WizDocumentListViewItem.cpp

    gui/documentviewer/WizDocumentView.cpp
    gui/documentviewer/WizDocumentWebView.cpp
    gui/documentviewer/WizSingleDocumentView.cpp
    gui/documentviewer/WizDocumentEditStatus.cpp
    gui/documentviewer/WizTitleBar.cpp
    gui/documentviewer/WizEditorToolBar.cpp
    gui/documentviewer/WizEditorInsertLinkForm.cpp
    gui/documentviewer/WizEditorInsertTableForm.cpp
    gui/documentviewer/WizNoteInfoForm.cpp
    gui/documentviewer/WizSearchReplaceWidget.cpp
    gui/documentviewer/WizTitleEdit.cpp
    gui/documentviewer/WizSvgEditorDialog.cpp
    gui/documentviewer/CollaborationDocView.cpp
    gui/documentviewer/DocumentLoaderSaver.cpp
    gui/documentviewer/AbstractDocumentView.cpp

    WizActions.cpp
    WizNoteStyle.cpp
    WizLineInputDialog.cpp
    WizDocumentHistory.cpp
    WizAttachmentListWidget.cpp
    WizTagListWidget.cpp
    WizConsoleDialog.cpp
    WizPreferenceDialog.cpp
    WizProxyDialog.cpp
    WizCreateAccountDialog.cpp
    WizMainWindow.cpp
    WizCellButton.cpp
    WizInfoBar.cpp
    WizNotifyBar.cpp
    WizStatusBar.cpp
    WizUpgradeNotifyDialog.cpp
    WizUserCipherForm.cpp
    WizSearchWidget.cpp
    WizUpgrade.cpp
    WizButton.cpp
    WizFileImporter.cpp
    WizFileExporter.cpp
    widgets/FileExportWizard.cpp
    WizProgressDialog.cpp
    WizDocumentSelectionView.cpp
    WizDocumentTransitionView.cpp
    WizPopupButton.cpp
    WizWebSettingsDialog.cpp
    WizUserVerifyDialog.cpp
    WizMessageListView.cpp
    WizThumbCache.cpp
    WizMessageCompleter.cpp
    WizLoginDialog.cpp
    WizDocTemplateDialog.cpp
    WizMobileFileReceiver.cpp
    WizWebEngineInjectObject.cpp
    WizCombineNotesDialog.cpp

    WizOEMSettings.cpp    
    core/WizAccountManager.cpp
    core/WizCommentManager.cpp
    core/WizNoteManager.cpp
    share/WizThreads.cpp
    WizPositionDelegate.cpp
    main.cpp
    WizInitBizCertDialog.cpp
    share/WizDocumentStyle.cpp

    api/ApiWizHtmlEditorApp.cpp
    api/ApiWizExplorerWindow.cpp
    api/ApiWizCategoryCtrl.cpp
    api/ApiWizDocumentListCtrl.cpp
    api/ApiWizDatabase.cpp
    api/ApiWizExplorerApp.cpp
    api/QtClassWrapper.cpp

    utils/ExternalEditorLauncher.cpp
    widgets/FileLineEdit.cpp
    widgets/DownloadManagerWidget.cpp
    widgets/ShadowWidget.cpp
)

set(wiznote_HEADERS
    share/WizZip.h
    share/WizQtHelper.h
    share/WizMisc.h
    share/WizObject.h
    share/WizKMCore.h

    database/WizDatabase.h
    database/WizIndex.h
    database/WizIndexBase.h
    database/WizThumbIndex.h
    database/WizSyncableDatabase.h
    database/sqlite3ext.h
    database/sqlite3.h
    database/cppsqlite3.h
    database/WizDatabaseManager.h
    database/WizOnlineDatabase.h

    share/WizSettings.h
    share/WizUI.h
    share/WizHtml2Zip.h
    share/WizMd5.h
    share/WizWin32Helper.h
    share/WizDrawTextHelper.h
    share/WizUIHelper.h
    share/WizCommonUI.h
    share/WizAnimateAction.h
    share/WizPopupWidget.h
    share/WizMultiLineListWidget.h
    share/WizFileIconProvider.h
    share/WizWindowsHelper.h
    share/WizEnc.h
    share/WizZiwReader.h
    share/WizGlobal.h
    html/WizHtmlReader.h
    html/WizHtmlCollector.h
    html/WizHtmlTool.h
    html/WizGumboHelper.h
    sync/WizKMServer.h
    sync/WizKMSync.h
    sync/WizAsyncApi.h
    sync/WizAvatarUploader.h
    share/WizSearch.h
    widgets/WizAboutDialog.h
    widgets/WizSegmentedButton.h
    widgets/WizUserInfoWidget.h
    widgets/WizUserInfoWidgetBase.h
    widgets/WizScrollBar.h
    widgets/WizExternalEditorSettingDialog.h
    sync/WizToken.h
    sync/WizToken_p.h
    sync/WizApiEntry.h
    share/WizObjectDataDownloader.h
    sync/WizAvatarHost.h
    sync/WizAvatarHost_p.h
    utils/WizStyleHelper.h
    utils/WizPathResolve.h
    share/WizWindowTitleBar.h
    share/WizFramelessWindow.h
    share/WizShadowEffect.h
    widgets/WizTableSelector.h

    share/jsoncpp/json/json.h
    share/jsoncpp/json/json-forwards.h
    share/WizRequest.h

    jsplugin/JSPlugin.h
    jsplugin/JSPluginManager.h
    jsplugin/JSPluginSpec.h
    jsplugin/JSPluginWidgets.h
    jsplugin/JSRepl.h

    api/PublicAPIsServer.h
    api/WebSocketClientWrapper.h
    api/WebSocketTransport.h

    gui/tabbrowser/WizMainTabBrowser.h
    gui/tabbrowser/AbstractTabPage.h
    gui/tabbrowser/FullScreenWindow.h
    gui/tabbrowser/FullScreenNotification.h
    gui/tabbrowser/WebEngineWindow.h
    gui/tabbrowser/WizWebsiteView.h
    gui/tabbrowser/TabButton.h

    WizDef.h
    WizActions.h
    WizNoteStyle.h

    gui/categoryviewer/WizCategoryView.h
    gui/categoryviewer/WizCategoryViewItemBase.h
    gui/categoryviewer/WizCategoryViewSectionItem.h
    gui/categoryviewer/WizCategoryViewMessageItem.h
    gui/categoryviewer/WizCategoryViewShortcutItem.h
    gui/categoryviewer/WizCategoryViewSearchItem.h
    gui/categoryviewer/WizCategoryViewFolderItem.h
    gui/categoryviewer/WizCategoryViewTagItem.h
    gui/categoryviewer/WizCategoryViewGroupItem.h
    gui/categoryviewer/WizCategoryViewItem.h
    gui/categoryviewer/WizFolderView.h
    gui/categoryviewer/WizFolderSelector.h

    WizDevToolsDialog.h

    gui/doclistviewer/WizDocumentListView.h
    gui/doclistviewer/WizDocumentListViewBaseItem.h
    gui/doclistviewer/WizDocumentListViewDocumentItem.h
    gui/doclistviewer/WizDocumentListViewSectionItem.h
    gui/doclistviewer/WizDocumentListViewItem.h

    gui/documentviewer/WizDocumentView.h
    gui/documentviewer/WizDocumentWebView.h
    gui/documentviewer/WizSingleDocumentView.h
    gui/documentviewer/WizDocumentEditStatus.h
    gui/documentviewer/WizTitleBar.h
    gui/documentviewer/WizEditorToolBar.h
    gui/documentviewer/WizEditorInsertLinkForm.h
    gui/documentviewer/WizEditorInsertTableForm.h
    gui/documentviewer/WizNoteInfoForm.h
    gui/documentviewer/WizSearchReplaceWidget.h
    gui/documentviewer/WizTitleEdit.h
    gui/documentviewer/WizSvgEditorDialog.h
    gui/documentviewer/CollaborationDocView.h
    gui/documentviewer/DocumentLoaderSaver.h
    gui/documentviewer/AbstractDocumentView.h

    WizDocumentHistory.h
    WizAttachmentListWidget.h
    WizTagListWidget.h
    WizConsoleDialog.h
    WizPreferenceDialog.h
    WizProxyDialog.h
    WizCreateAccountDialog.h
    WizMainWindow.h
    WizCellButton.h
    WizInfoBar.h
    WizNotifyBar.h
    sync/WizSync.h
    sync/WizKMSync_p.h
    WizStatusBar.h
    WizUpgradeNotifyDialog.h
    WizUserCipherForm.h
    WizLineInputDialog.h
    WizSearchWidget.h
    WizUpgrade.h
    WizWebSettingsDialog.h
    WizPopupButton.h
    WizDocumentTransitionView.h
    WizButton.h
    WizFileImporter.h
    WizFileExporter.h
    widgets/FileExportWizard.h
    WizProgressDialog.h
    WizDocumentSelectionView.h
    WizUserVerifyDialog.h
    utils/WizLogger.h
    utils/WizMisc.h
    utils/WizPinyin.h
    utils/WizNotify.h
    WizMessageListView.h
    WizThumbCache.h
    WizThumbCache_p.h
    WizMessageCompleter.h
    widgets/WizImageButton.h
    WizLoginDialog.h
    share/WizFileMonitor.h
    widgets/WizCodeEditorDialog.h
    WizDocTemplateDialog.h
    WizMobileFileReceiver.h
    widgets/WizFramelessWebDialog.h
    share/WizLockedFile.h
    share/WizLocalPeer.h
    share/WizSingleApplication.h
    widgets/WizScreenShotWidget.h
    widgets/WizVerificationCodeDialog.h
    widgets/WizEmailShareDialog.h
    share/WizWebEngineView.h
    WizWebEngineInjectObject.h
    widgets/WizShareLinkDialog.h
    share/WizAnalyzer.h
    share/WizTranslater.h
    widgets/WizAdvancedSearchDialog.h
    share/WizUdpClient.h
    share/WizMessageBox.h
    WizOEMSettings.h
    widgets/WizTagBar.h
    widgets/WizTrayIcon.h
    share/WizEventLoop.h
    sync/WizJSONServerBase.h
    share/WizObjectOperator_p.h
    share/WizObjectOperator.h
    widgets/WizLocalProgressWebView.h
    widgets/WizCrashReportDialog.h
    widgets/WizCustomToolBar.h
    widgets/WizTipsWidget.h
    widgets/WizTemplatePurchaseDialog.h
    widgets/WizExecutingActionDialog.h
    widgets/WizUserServiceExprDialog.h
    WizPositionDelegate.h
    core/WizAccountManager.h
    core/WizCommentManager.h
    core/WizNoteManager.h
    share/WizThreads.h
    share/WizThreads_p.h
    WizInitBizCertDialog.h
    share/WizDocumentStyle.h
    WizCombineNotesDialog.h

    api/ApiWizHtmlEditorApp.h
    api/ApiWizExplorerWindow.h
    api/ApiWizCategoryCtrl.h
    api/ApiWizDocumentListCtrl.h
    api/ApiWizDatabase.h
    api/ApiWizExplorerApp.h
    api/QtClassWrapper.h

    utils/ExternalEditorLauncher.h
    widgets/FileLineEdit.h
    widgets/DownloadManagerWidget.h
    widgets/ShadowWidget.h
)

set(wiznote_FORMS
    ui/WizLineInputDialog.ui
    ui/WizConsoleDialog.ui
    ui/WizPreferenceDialog.ui
    ui/WizProxyDialog.ui
    ui/WizCreateAccountDialog.ui
    ui/WizUpgradeNotifyDialog.ui
    ui/WizUserCipherForm.ui
    ui/WizNoteInfoForm.ui
    ui/WizEditorInsertLinkForm.ui
    ui/WizEditorInsertTableForm.ui
    ui/WizProgressDialog.ui
    ui/WizLoginDialog.ui
    ui/WizSearchReplaceWidget.ui
    ui/WizDocTemplateDialog.ui
    ui/WizVerificationCodeDialog.ui
    ui/WizEmailShareDialog.ui
    ui/WizAdvancedSearchDialog.ui
    ui/WizCrashReportDialog.ui
    ui/WizTemplatePurchaseDialog.ui
    ui/WizExecutingActionDialog.ui
    ui/WizInitBizCertDialog.ui
    ui/WizUserServiceExprDialog.ui
    ui/WizCombineNotesDialog.ui
    ui/WizExternalEditorSettingDialog.ui
    ui/WizExternalEditorInfoDialog.ui
    ui/WizFileExportDialog.ui
    ui/DownloadManagerWidget.ui
    ui/DownloadWidget.ui
)

set(wiznote_TRANSLATIONS
    ${CMAKE_SOURCE_DIR}/i18n/wiznote_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/i18n/wiznote_zh_TW.ts
)

set(wiznote_RCS
    ${CMAKE_SOURCE_DIR}/resources/wiznote.qrc
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${cryptopp_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIRS}
    libs/3rdparty
    libs/3rdparty/quzip
)

#============================================================================
# Compilation
#============================================================================

# Preprocess Qt related files.
include(QtChooser)
qt_add_ui(wiznote_FORM_HEADERS ${wiznote_FORMS})
qt_add_resources(wiznote_RC ${wiznote_RCS})
qt_add_translation(wiznote_QM ${wiznote_TRANSLATIONS})

message("WizNotePlus whill compiling as ${CMAKE_BUILD_TYPE}, use Qt: ${QT_VERSION}")

# Monitor file content changes in share directory
file(GLOB_RECURSE share_files LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/share/**/*)
foreach(filename IN LISTS share_files)
    string(REPLACE "${CMAKE_SOURCE_DIR}/share" "${WIZNOTE_RESOURCE_DIR}" dest_filename "${filename}")
    configure_file(${filename} ${dest_filename} COPYONLY)
endforeach(filename IN LISTS share_files)

# build
if(UNIX)
    if(APPLE)
        qt_add_ui(wiznote_FORM_HEADERS ${wiznote_FORMS_MAC})

        set(wiznote_LOGO ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote.icns)

        # we should copy qt internal qm files, otherwise qt can't translate some internal resources
        # and also macdeployqt will not do this work for us
        # refer to: https://bugreports.qt-project.org/browse/QTBUG-18662
        file(GLOB _qt5_QM ${qt_translations_dir}/qt_*.qm)

        add_executable(WizNotePlus MACOSX_BUNDLE  ${wiznote_HEADERS} ${wiznote_HEADERS_MAC} ${wiznote_FORM_HEADERS} ${wiznote_SOURCES_MAC} ${wiznote_SOURCES} ${wiznote_QM} ${_qt5_QM} ${wiznote_RC} ${wiznote_LOGO})
        set_target_properties(WizNotePlus PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_MAC")

        # this should be issue of cmake, refer: http://public.kitware.com/Bug/bug_revision_view_page.php?rev_id=958
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-F /System/Library/Frameworks")

        # qmake will do this automatically
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mmacosx-version-min=10.7")

        # suppress cmake warnning when link library is not a target, refer to cmp0016 for detailed info.
        #cmake_policy(SET CMP0016 OLD)
        find_library(_CARBON_LIBRARY Carbon)
        find_library(_COCOA_LIBRARY Cocoa)
        find_library(_WEBENGINE_LIBRARY WebEngine)
        find_library(_WEBENGINEWIDGETS_LIBRARY WebEngineWidgets)
        find_library(_STORE_LIBRARY StoreKit)
        find_library(_IO_LIBRARY IOKit)
        if (PLCrashReporter)
            find_library(_CRASHREPORTER CrashReporter)
        endif(PLCrashReporter)

        # please set env: MACOSX_DEPLOYMENT_TARGET = 10.8 to ~/.bash_profile if you need NSUserNotification support.
        message("use cocoa library: ${_COCOA_LIBRARY}")
        target_link_libraries(${PROJECT_NAME} ${_CARBON_LIBRARY} ${_COCOA_LIBRARY}
                  ${_STORE_LIBRARY} ${_IO_LIBRARY} ${_CRASHREPORTER})

        # setup plist info and logo
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/osx/Info.plist")
        set_source_files_properties(${wiznote_LOGO} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_source_files_properties(${wiznote_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/locales)
        set_source_files_properties(${_qt5_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/locales)

        # copy resources to bundle
        #set(wiznote_BUNDLE ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app)
        #set(wiznote_BUNDLE_CONTENTS ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app/Contents)

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/build/osx/localize/ ${WIZNOTE_RESOURCE_DIR})

    elseif(NOT APPLE)
        add_executable(WizNotePlus ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${wiznote_RC})
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_LINUX")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-static-libstdc++ -static-libgcc")

        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION share/wiznote PATTERN ".DS_Store" EXCLUDE)
        install(FILES ${wiznote_QM} DESTINATION share/wiznote/locales)
        install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/licenses/wiznote)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/wiznote.desktop DESTINATION share/applications)

        foreach(ICON_SIZE 8 16 24 32 48 64 128 256 512)
            install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote${ICON_SIZE}.png DESTINATION share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps RENAME wiznote.png)
        endforeach(ICON_SIZE)

        foreach(_file ${wiznote_QM})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/wiznote/locales/
                COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${CMAKE_BINARY_DIR}/share/wiznote/locales/)
        endforeach()
    endif()
else()
    # Windows
    add_definitions(-DUNICODE=TRUE)
    add_definitions(-D_UNICODE=TRUE)
    add_definitions(-DNOMINMAX)
    #add_compile_options("/utf-8") # UNICODE or _UNICODE may not work
    add_executable(WizNotePlus WIN32 ${wiznote_SOURCES} ${wiznote_HEADERS} 
        ${wiznote_FORM_HEADERS} ${wiznote_QM} ${wiznote_RC} 
        ${CMAKE_SOURCE_DIR}/resources/logo.rc
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_WIN32")

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION share/wiznote PATTERN ".DS_Store" EXCLUDE)
    install(FILES ${wiznote_QM} DESTINATION share/wiznote/locales)
    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/licenses/wiznote)

    foreach(_file ${wiznote_QM})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/wiznote/locales/
            COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${CMAKE_BINARY_DIR}/share/wiznote/locales/)
    endforeach()
endif()

qt_use_modules(WizNotePlus)
qt_suppress_warnings(WizNotePlus)

if (APPLE)
    include_directories(~/Library/Frameworks)
endif()


link_directories(
    ${CMAKE_BINARY_DIR}/bin
)

#============================================================================
# Linking libraries
#============================================================================

if(UNIX)
    if(APPLE)
        if(APPSTORE_BUILD)
            target_link_libraries(${PROJECT_NAME}
                ${CMAKE_DL_LIBS}
                ${cryptopp_LIBRARIES}
                ${ZLIB_LIBRARIES}
                ${Gumbo_LIBRARIES}
                FramelessHelper
                QuaZip
                gumbo
            )
        else(APPSTORE_BUILD)
            target_link_libraries(${PROJECT_NAME}
                ${CMAKE_DL_LIBS}
                ${cryptopp_LIBRARIES}
                ${ZLIB_LIBRARIES}
                ${Gumbo_LIBRARIES}
                FramelessHelper
                QuaZip
                gumbo
            )
        endif(APPSTORE_BUILD)
    else(APPLE)
        # Linux: linking order is important.
        target_link_libraries(${PROJECT_NAME}
            -lpthread
            X11
            ${CMAKE_DL_LIBS}
            ${cryptopp_LIBRARIES}
            ${OpenSSL_LIBRARIES}
            ${ZLIB_LIBRARIES}
            ${Gumbo_LIBRARIES}
            wangwenx190::FramelessHelper
            QuaZip
            gumbo
        )
    endif(APPLE)
else(NOT UNIX)
    target_link_libraries(${PROJECT_NAME}
        ${CMAKE_DL_LIBS}
        ${cryptopp_LIBRARIES}
        ${OpenSSL_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${Gumbo_LIBRARIES}
        FramelessHelper
        QuaZip
        dwmapi
        gumbo
    )
endif()
