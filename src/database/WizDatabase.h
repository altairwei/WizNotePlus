#ifndef WIZDATABASE_H
#define WIZDATABASE_H

#include <QPointer>
#include <QMap>
#include <QMutex>

#include "WizIndex.h"
#include "WizThumbIndex.h"
#include "WizSyncableDatabase.h"

#include "share/WizZiwReader.h"

class WizDatabase;
class WizFolder;
class WizDocument;
class WizDocumentAttachment;

class WizDocument : public QObject
{
    Q_OBJECT

public:
    WizDocument(WizDatabase& db, const WIZDOCUMENTDATA& data, QObject *parent = nullptr);

    QString GUID() const { return m_data.strGUID; }
    Q_PROPERTY(QString GUID READ GUID)

    QString Title() const { return m_data.strTitle; }
    bool setTitle(const QString &strTitle);
    Q_PROPERTY(QString Title READ Title WRITE setTitle NOTIFY TitleChanged)

    QString Author() const { return m_data.strAuthor; }
    Q_PROPERTY(QString Author READ Author NOTIFY AuthorChanged)

    QString Keywords() const { return m_data.strKeywords; }
    Q_PROPERTY(QString Keywords READ Keywords NOTIFY KeywordsChanged)

    QString Name() const { return m_data.strName; }
    Q_PROPERTY(QString Name READ Name NOTIFY NameChanged)

    QString Location() const { return m_data.strLocation; }
    Q_PROPERTY(QString Location READ Location NOTIFY LocationChanged)

    QString URL() const { return m_data.strURL; }
    Q_PROPERTY(QString URL READ URL NOTIFY URLChanged)

    QString Type() const { return m_data.strType; }
    Q_PROPERTY(QString Type READ Type NOTIFY TypeChanged)

    QString Owner() const { return m_data.strOwner; }
    Q_PROPERTY(QString Owner READ Owner NOTIFY OwnerChanged)

    QString FileType() const { return m_data.strFileType; }
    Q_PROPERTY(QString FileType READ FileType NOTIFY FileTypeChanged)

    QString FileName() const;
    Q_PROPERTY(QString FileName READ FileName)

    bool Downloaded() const;
    Q_PROPERTY(QString Downloaded READ Downloaded NOTIFY DownloadedChanged)

    long ReadCount() const { return m_data.nReadCount; }
    Q_PROPERTY(long ReadCount READ ReadCount NOTIFY ReadCountChanged)

    long AttachmentCount() const { return m_data.nAttachmentCount; }
    Q_PROPERTY(long AttachmentCount READ AttachmentCount NOTIFY AttachmentCountChanged)

    QString DateCreated() const { return m_data.tCreated.toString(Qt::ISODate); }
    Q_PROPERTY(QString DateCreated READ DateCreated NOTIFY DateCreatedChanged)

    QString DateModified() const { return m_data.tModified.toString(Qt::ISODate); }
    bool setDateModified(const QString &strDateModified);
    Q_PROPERTY(QString DateModified READ DateModified WRITE setDateModified NOTIFY DateModifiedChanged)

    QString DateAccessed() const { return m_data.tAccessed.toString(Qt::ISODate); }
    Q_PROPERTY(QString DateAccessed READ DateAccessed NOTIFY DateAccessedChanged)

    QString DataDateModified() const { return m_data.tDataModified.toString(Qt::ISODate); }
    Q_PROPERTY(QString DataDateModified READ DataDateModified NOTIFY DataDateModifiedChanged)

    QString DataMD5() const { return m_data.strDataMD5; }
    Q_PROPERTY(QString DataMD5 READ DataMD5 NOTIFY DataMD5Changed)

    QObject *Database() const;
    Q_PROPERTY(QObject *Database READ Database)

    bool isProtected() const { return m_data.nProtected; }
    bool encryptDocument() { return false; }
    WizDatabase &db() { return m_db; }

    void makeSureObjectDataExists();

    QString getAttachmentsPath(bool create);
    bool isInDeletedItemsFolder();
    bool moveTo(WizFolder* pFolder);
    bool moveTo(WizDatabase& targetDB, WizFolder* pFolder);
    bool moveTo(WizDatabase& targetDB, const WIZTAGDATA& targetTag);
    bool copyTo(WizDatabase& targetDB, WizFolder* pFolder, bool keepDocTime,
                bool keepDocTag, QString& newDocGUID);
    bool copyTo(WizDatabase& targetDB, const WIZTAGDATA& targetTag, bool keepDocTime);
    bool addTag(const WIZTAGDATA& dataTag);
    bool removeTag(const WIZTAGDATA& dataTag);

    WIZDOCUMENTDATA data() { return m_data; }

signals:
    void TitleChanged();
    void AuthorChanged();
    void KeywordsChanged();
    void NameChanged();
    void LocationChanged();
    void URLChanged();
    void TypeChanged();
    void OwnerChanged();
    void FileTypeChanged();
    void DownloadedChanged();
    void ReadCountChanged();
    void AttachmentCountChanged();
    void DateCreatedChanged();
    void DateModifiedChanged();
    void DateAccessedChanged();
    void DataDateModifiedChanged();
    void DataMD5Changed();

public:
    Q_INVOKABLE void Delete();
    Q_INVOKABLE void PermanentlyDelete();
    Q_INVOKABLE void moveTo(QObject* pFolder);
    Q_INVOKABLE bool UpdateDocument3(const QString& strHtml, int nFlags);
    Q_INVOKABLE bool UpdateDocument4(const QString& strHtml, const QString& strURL, int nFlags);
    Q_INVOKABLE bool UpdateDocument6(const QString &strHtmlFileName, const QString &strURL, int nFlags);
    Q_INVOKABLE void deleteToTrash();
    Q_INVOKABLE void deleteFromTrash();

    Q_INVOKABLE bool SaveToFolder(const QString &folderName);

    Q_INVOKABLE void Reload();

    Q_INVOKABLE QString GetParamValue(const QString &strParamName);
    Q_INVOKABLE bool SetParamValue(const QString &strParamName, const QString &strNewValue);
    Q_INVOKABLE bool RemoveParam(const QString &strParamName);

    Q_INVOKABLE void Close();

private:
    bool copyDocumentTo(const QString &sourceGUID, WizDatabase &targetDB, const QString &strTargetLocation,
                        const WIZTAGDATA &targetTag, QString &resultGUID, bool keepDocTime, bool keepOtherInfo = true);
    bool copyDocumentAttachment(const WIZDOCUMENTDATA& sourceDoc, WizDatabase& targetDB,
                                WIZDOCUMENTDATA& targetDoc);
    bool reloadDocumentInfo();

private:
    WizDatabase& m_db;
    WIZDOCUMENTDATA m_data;

};


class WizDocumentAttachment : public QObject
{
    Q_OBJECT

public:
    WizDocumentAttachment(WizDatabase &db, 
        const WIZDOCUMENTATTACHMENTDATA &data, QObject *parent = nullptr);

    QString GUID() const { return m_data.strGUID; }
    Q_PROPERTY(QString GUID READ GUID)

    QString Name() const { return m_data.strName; }
    Q_PROPERTY(QString Name READ Name NOTIFY NameChanged)

    QString Description() const { return m_data.strDescription; }
    Q_PROPERTY(QString Description READ Description NOTIFY DescriptionChanged)

    QString URL() const { return m_data.strURL; }
    Q_PROPERTY(QString URL READ URL NOTIFY URLChanged)

    QString InfoDateModified() const { return m_data.tInfoModified.toString(Qt::ISODate); }
    Q_PROPERTY(QString InfoDateModified READ InfoDateModified NOTIFY InfoDateModifiedChanged)

    QString InfoMD5() const { return m_data.strInfoMD5; }
    Q_PROPERTY(QString InfoMD5 READ InfoMD5 NOTIFY InfoMD5Changed)

    QString DataDateModified() const { return m_data.tDataModified.toString(Qt::ISODate); }
    Q_PROPERTY(QString DataDateModified READ DataDateModified NOTIFY DataDateModifiedChanged)
    
    QString DataMD5() const { return m_data.strDataMD5; }
    Q_PROPERTY(QString DataMD5 READ DataMD5 NOTIFY DataMD5Changed)

    QString DocumentGUID() const { return m_data.strDocumentGUID; }
    Q_PROPERTY(QString DocumentGUID READ DocumentGUID NOTIFY DocumentGUIDChanged)


    WIZDOCUMENTATTACHMENTDATA data() { return m_data; }

signals:
    void NameChanged();
    void DescriptionChanged();
    void URLChanged();
    void InfoDateModifiedChanged();
    void InfoMD5Changed();
    void DataDateModifiedChanged();
    void DataMD5Changed();
    void DocumentGUIDChanged();

private:
    WizDatabase& m_db;
    WIZDOCUMENTATTACHMENTDATA m_data;

};

class WizFolder : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString location READ location NOTIFY locationChanged)

public:
    WizFolder(WizDatabase& db, const QString& strLocation, QObject *parent = nullptr);

    bool isDeletedItems() const;
    bool isInDeletedItems() const;

    // used for move and rename folder
    void moveToLocation(const QString& strDestLocation);

    static bool canMove(const QString& strSource, const QString& strDest);
    bool canMove(WizFolder* pSrc, WizFolder* pDest) const;

    Q_INVOKABLE void Delete();
    Q_INVOKABLE void moveTo(QObject* dest);
    Q_INVOKABLE QString location() const { return m_strLocation; }
    Q_INVOKABLE QObject* CreateDocument2(const QString& strTitle, const QString& strURL);

    Q_INVOKABLE void Close();

protected:
    WizDatabase& m_db;
    CString m_strLocation;

Q_SIGNALS:
    void moveDocument(int nTotal, int nProcessed,
                      const QString& strOldLocation,
                      const QString& strNewLocation,
                      const WIZDOCUMENTDATA& data);

    void locationChanged();
};

/*
 * High level operation layer of database
 */
class WizDatabase
        : public WizIndex
        , public WizThumbIndex
        , public IWizSyncableDatabase
{
    Q_OBJECT

private:
    QString m_strAccountFolderName;
    // all databases share one user id, user id data only stored in personal databases
    static QString m_strUserId;
    QString m_strPassword;
    WIZDATABASEINFO m_info;
    QPointer<WizZiwReader> m_ziwReader;

    bool m_bIsPersonal;
    QMap<QString, WizDatabase*> m_mapGroups;
    QMutex m_mtxTempFile;

private:
    QMutex m_mutexCache;
    CWizGroupDataArray m_cachedGroups;
    CWizBizDataArray m_cachedBizs;

public:
    WizDatabase();

    const WIZDATABASEINFO& info() { return m_info; }
    QString name() const { return m_info.name; }
    QString bizGuid() const { return m_info.bizGUID; }
    int permission() const { return m_info.nPermission; }

    // START: IWizSyncableDatabase interface implementations
    //-------------------------------------------------------------------

    virtual QString getUserId();
    virtual QString getUserGuid();
    virtual QString getPassword();

    virtual bool saveLastSyncTime();
    virtual WizOleDateTime getLastSyncTime();

    bool WizDayOnce(const QString& strName);

    // versions
    virtual qint64 getObjectVersion(const QString& strObjectType);
    virtual bool setObjectVersion(const QString& strObjectType, qint64 nVersion);

    virtual qint64 getObjectLocalVersion(const QString& strObjectGUID,
                                         const QString& strObjectType);
    virtual qint64 getObjectLocalVersionEx(const QString& strObjectGUID,
                                         const QString& strObjectType,
                                           bool& bObjectVersion);
    virtual bool setObjectLocalServerVersion(const QString& strObjectGUID,
                                             const QString& strObjectType,
                                             qint64 nVersion);
    virtual void onObjectUploaded(const QString &strObjectGUID, const QString &strObjectType);

    // query
    virtual bool getModifiedDeletedList(CWizDeletedGUIDDataArray& arrayData);
    virtual bool getModifiedTagList(CWizTagDataArray& arrayData);
    virtual bool getModifiedStyleList(CWizStyleDataArray& arrayData);
    virtual bool getModifiedParamList(CWizDocumentParamDataArray& arrayData);
    virtual bool getModifiedDocumentList(CWizDocumentDataArray& arrayData);
    virtual bool getModifiedAttachmentList(CWizDocumentAttachmentDataArray& arrayData);
    virtual bool getModifiedMessageList(CWizMessageDataArray& arrayData);
    virtual bool getObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayObject);

    virtual bool documentFromGuid(const QString& strGUID,
                                  WIZDOCUMENTDATA& dataExists);

    // download
    virtual bool onDownloadDeletedList(const CWizDeletedGUIDDataArray& arrayData);
    virtual bool onDownloadTagList(const CWizTagDataArray& arrayData);
    virtual bool onDownloadStyleList(const CWizStyleDataArray& arrayData);
    virtual bool onDownloadDocumentList(const CWizDocumentDataArray& arrayData);
    virtual bool onDownloadAttachmentList(const CWizDocumentAttachmentDataArray& arrayData);
    virtual bool onDownloadMessageList(const CWizMessageDataArray& arrayData);
    virtual bool onDownloadParamList(const CWizDocumentParamDataArray& arrayData);

    virtual bool updateObjectData(const QString& strDisplayName,
                                  const QString& strObjectGUID,
                                  const QString& strObjectType,
                                  const QByteArray& stream);

    virtual bool isObjectDataDownloaded(const QString& strGUID,
                                        const QString& strType);
    virtual bool setObjectDataDownloaded(const QString& strGUID,
                                         const QString& strType,
                                         bool downloaded);

    virtual bool setObjectServerDataInfo(const QString& strGUID,
                                         const QString& strType,
                                         WizOleDateTime& tServerDataModified,
                                         const QString& strServerMD5);

    // upload
    virtual bool initDocumentData(const QString& strGUID,
                                  WIZDOCUMENTDATAEX& data,
                                  bool forceUploadData);

    virtual bool initAttachmentData(const QString& strGUID,
                                    WIZDOCUMENTATTACHMENTDATAEX& data);

    virtual bool onUploadObject(const QString& strGUID,
                                const QString& strObjectType);
    virtual bool onUploadParam(const QString& strDocumentGuid, const QString& strName);


    // modify
    virtual bool modifyMessagesLocalChanged(CWizMessageDataArray &arrayData);

    // info and groups
    virtual void setUserInfo(const WIZUSERINFO& info);
    virtual void setKbInfo(const QString& strKBGUID, const WIZKBINFO& info);
    virtual QString getGroupName();
    virtual WIZGROUPDATA getGroupInfo();
    virtual bool onDownloadGroups(const CWizGroupDataArray& arrayGroup);
    virtual bool onDownloadBizs(const CWizBizDataArray& arrayBiz);
    virtual bool onDownloadBizUsers(const QString& kbGuid, const CWizBizUserDataArray& arrayUser);
    virtual IWizSyncableDatabase* getGroupDatabase(const WIZGROUPDATA& group);
    virtual void closeGroupDatabase(IWizSyncableDatabase* pDatabase);
    virtual IWizSyncableDatabase* getPersonalDatabase();

    virtual bool isGroup();
    bool isPersonalGroup();
    virtual bool hasBiz();

    virtual bool isGroupAdmin();
    virtual bool isGroupSuper();
    virtual bool isGroupEditor();
    virtual bool isGroupAuthor();
    virtual bool isGroupReader();
    virtual bool isGroupOwner();

    virtual bool canEditDocument(const WIZDOCUMENTDATA& data);
    virtual bool canEditAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);

    // obsolote
    virtual qint64 getObjectLocalServerVersion(const QString& strObjectGUID,
                                               const QString& strObjectType);

    virtual bool createConflictedCopy(const QString& strObjectGUID,
                                      const QString& strObjectType);

    virtual long getLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType);
    virtual bool setLocalFlags(const QString& strObjectGUID,
                               const QString& strObjectType,
                               long flags);

    // key-value api based syncing, user config, folders, biz users list etc.
    virtual void getAccountKeys(CWizStdStringArray& arrayKey);
    virtual qint64 getAccountLocalValueVersion(const QString& strKey);
    virtual void setAccountLocalValue(const QString& strKey,
                                      const QString& strValue,
                                      qint64 nServerVersion,
                                      bool bSaveVersion);

    virtual void getKBKeys(CWizStdStringArray& arrayKey);
    virtual qint64 getLocalValueVersion(const QString& strKey);
    virtual QString getLocalValue(const QString& strKey);
    virtual void setLocalValueVersion(const QString& strKey,
                                      qint64 nServerVersion);
    virtual void setLocalValue(const QString& strKey,
                               const QString& strValue,
                               qint64 nServerVersion,
                               bool bSaveVersion);

    virtual bool processValue(const QString& strKey);

    virtual void getAllBizUserIds(CWizStdStringArray& arrayText);
    virtual bool getAllBizUsers(CWizBizUserDataArray& arrayUser);

    virtual void clearLastSyncError();
    virtual QString getLastSyncErrorMessage();
    virtual void onTrafficLimit(const QString& strErrorMessage);
    virtual void onStorageLimit(const QString& strErrorMessage);
    virtual void onNoteCountLimit(const QString& strErrorMessage);
    virtual void onBizServiceExpr(const QString& strBizGUID, const QString& strErrorMessage);
    virtual bool isTrafficLimit();
    virtual bool isStorageLimit();
    virtual bool isNoteCountLimit();
    virtual bool isBizServiceExpr(const QString& strBizGUID);
    virtual bool getStorageLimitMessage(QString& strErrorMessage);
    virtual bool getTrafficLimitMessage(QString& strErrorMessage);
    virtual bool getNoteCountLimit(QString& strErrorMessage);

    virtual bool setMeta(const QString& strSection, const QString& strKey, const QString& strValue);
    virtual QString meta(const QString& strSection, const QString& strKey);

    // end interface implementations
    bool onDownloadDocument(const WIZDOCUMENTDATAEX& data);

    // helper methods for interface
    void setObjectSyncTimeLine(int nDays);
    int getObjectSyncTimeline();
    void setDownloadAttachmentsAtSync(bool download);
    bool getDownloadAttachmentsAtSync();
    bool isFolderExists(const QString& folder);
    QString getFolders();
    QString getFoldersPos();
    QString getGroupTagsPos();
    void setFoldersPos(const QString& foldersPos, qint64 nVersion);
    void setFolders(const QString& strFolders, qint64 nVersion, bool bSaveVersion);
    void setGroupTagsPos(const QString& tagsPos, qint64 nVersion);
    QString getFavorites();
    void setFavorites(const QString& favorites, qint64 nVersion);

    void setFoldersPosModified();
    void setGroupTagsPosModified();

    virtual bool getAllNotesOwners(CWizStdStringArray &arrayOwners);

    virtual bool deleteDocumentFromLocal(const QString& strDocumentGuid);
    virtual bool deleteAttachmentFromLocal(const QString& strAttachmentGuid);

    // END: IWizSyncableDatabase interface implementations
    //-------------------------------------------------------------------

public:
    bool open(const QString& strAccountFolderName, const QString& strKbGUID = NULL);
    bool loadDatabaseInfo();
    bool setDatabaseInfo(const WIZDATABASEINFO& dbInfo);
    bool initDatabaseInfo(const WIZDATABASEINFO& dbInfo);
    bool getUserInfo(WIZUSERINFO& userInfo);

    // when upgrade app, remove or update invalid data
    void updateInvalidData();

    // path resolve
    QString getAccountPath() const;
    QString getAccountFolderName() const;

    QString getDataPath() const;
    QString getIndexFileName() const;
    QString getThumbFileName() const;
    QString getDocumentsDataPath() const;
    QString getAttachmentsDataPath() const;
    virtual QString getDocumentFileName(const QString& strGUID) const;
    QString getAttachmentFileName(const QString& strGUID);
    QString getAvatarPath() const;
    QString getDefaultNoteLocation() const;
    QString getDocumentAuthorAlias(const WIZDOCUMENTDATA& doc);
    QString getDocumentOwnerAlias(const WIZDOCUMENTDATA& doc);

    bool getUserName(QString& strUserName);
    bool setUserName(const QString& strUserName);
    bool getUserDisplayName(QString& strDisplayName);

    //QString getPassword() const { return m_strPassword; }
    QString getUserAlias();
    WizDatabase* personalDatabase();


    QString getEncryptedPassword();
    bool getPassword(CString& strPassword);
    bool setPassword(const QString& strPassword, bool bSave = false);
    UINT getPasswordFalgs();
    bool setPasswordFalgs(UINT nFlags);

    bool setUserCert(const QString& strN, const QString& stre, const QString& strd, const QString& strHint);
    bool getUserCert(QString& strN, QString& stre, QString& strd, QString& strHint);

    bool getAllGroupInfo(CWizGroupDataArray& arrayGroup);
    bool setAllGroupInfo(const CWizGroupDataArray& arrayGroup);

    bool getAllBizInfo(CWizBizDataArray& arrayBiz);
    bool setAllBizInfo(const CWizBizDataArray& arrayBiz);

    bool getBizData(const QString& bizGUID, WIZBIZDATA& biz);
    bool getBizGuid(const QString& strGroupGUID, QString& strBizGUID);
    bool getGroupData(const QString& groupGUID, WIZGROUPDATA& group);
    QString getKbServer(const QString &kbGuid);

    static bool isEmptyBiz(const CWizGroupDataArray& arrayGroup, const QString& bizGUID);
    static bool getOwnGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayOwnGroup);
    static bool getJionedGroups(const CWizGroupDataArray& arrayAllGroup, CWizGroupDataArray& arrayJionedGroup);

    bool updateBizUser(const WIZBIZUSER& user);
    bool updateMessage(const WIZMESSAGEDATA& msg);
    bool updateMessages(const CWizMessageDataArray& arrayMsg);
    bool updateDeletedGuid(const WIZDELETEDGUIDDATA& data);
    bool updateDeletedGuids(const CWizDeletedGUIDDataArray& arrayDeletedGUID);
    bool updateTag(const WIZTAGDATA& data);
    bool updateTags(const CWizTagDataArray &arrayTag);
    bool updateStyle(const WIZSTYLEDATA& data);
    bool updateStyles(const CWizStyleDataArray& arrayStyle);
    bool updateDocument(const WIZDOCUMENTDATAEX& data);
    bool updateDocuments(const std::deque<WIZDOCUMENTDATAEX>& arrayDocument);
    bool updateAttachment(const WIZDOCUMENTATTACHMENTDATAEX& data);
    bool updateAttachments(const CWizDocumentAttachmentDataArray& arrayAttachment);

    bool setDocumentFlags(const QString& strDocumentGuid, const QString& strFlags);

    bool updateDocumentData(WIZDOCUMENTDATA& data, const QString& strHtml,
                            const QString& strURL, int nFlags, bool notifyDataModify = true);
    bool updateDocumentDataWithFolder(WIZDOCUMENTDATA& data, const QString& strFolder,
                                          bool notifyDataModify = true);
    void clearUnusedImages(const QString& strHtml, const QString& strFilePath);

    bool updateDocumentAbstract(const QString& strDocumentGUID);
    bool updateDocumentAbstract(const QString &strDocGUID, const QString &strAbstract);

    virtual bool updateDocumentDataMD5(WIZDOCUMENTDATA& data, const CString& strZipFileName, bool notifyDataModify = true);

    // delete
    bool deleteObject(const QString &strGUID, const QString &strType, bool bLog);
    bool deleteTagWithChildren(const WIZTAGDATA& data, bool bLog);
    bool deleteAttachment(const WIZDOCUMENTATTACHMENTDATA& data, bool bLog,
                          bool bResetDocInfo, bool updateAttachList = true);
    bool deleteGroupFolder(const WIZTAGDATA& data, bool bLog);

    bool isDocumentModified(const CString& strGUID);
    bool isAttachmentModified(const CString& strGUID);
    bool isDocumentDownloaded(const CString& strGUID);
    bool isAttachmentDownloaded(const CString& strGUID);
    bool getAllObjectsNeedToBeDownloaded(CWizObjectDataArray& arrayData, int nTimeLine);
    bool updateSyncObjectLocalData(const WIZOBJECTDATA& data);

    CString getObjectFileName(const WIZOBJECTDATA& data);

    bool getDocumentsByTag(const WIZTAGDATA& tag, CWizDocumentDataArray& arrayDocument);

    //load zip data
    bool loadDocumentDecryptedData(const QString& strDocumentGUID, QByteArray& arrayData);
    //load raw data, for upload to server
    bool loadDocumentZiwData(const QString& strDocumentGUID, QByteArray& arrayData);

    bool loadFileData(const QString& strFileName, QByteArray& arrayData);
    bool writeDataToDocument(const QString& strDocumentGUID, const QByteArray &arrayData);
    bool loadAttachmentData(const CString& strDocumentGUID,
                            QByteArray& arrayData);
    bool loadCompressedAttachmentData(const QString& strGUID,
                                      QByteArray& arrayData);
    bool saveCompressedAttachmentData(const CString& strGUID,
                                      const QByteArray& arrayData);
    bool modifyAttachmentDataMd5(const QString& strGUID, const QString& md5);


    static CString getRootLocation(const CString& strLocation);
    static CString getLocationName(const CString& strLocation);
    static CString getLocationDisplayName(const CString& strLocation);
    static bool getAllRootLocations(const CWizStdStringArray& arrayAllLocation, \
                                    CWizStdStringArray& arrayLocation);
    static bool getChildLocations(const CWizStdStringArray& arrayAllLocation, \
                                  const QString& strLocation, \
                                  CWizStdStringArray& arrayLocation);

    static bool isInDeletedItems(const CString& strLocation);

    bool getDocumentTitleStartWith(const QString& titleStart, int nMaxCount, CWizStdStringArray& arrayTitle);

    QString getDocumentLocation(const WIZDOCUMENTDATA& doc);

    bool createDocumentAndInit(const CString& strHtml, \
                               const CString& strHtmlUrl, \
                               int nFlags, \
                               const CString& strTitle, \
                               const CString& strName, \
                               const CString& strLocation, \
                               const CString& strURL, \
                               WIZDOCUMENTDATA& data);

    bool createDocumentAndInit(const WIZDOCUMENTDATA& sourceDoc,  \
                               const QByteArray& baData, \
                               const QString& strLocation, \
                               const WIZTAGDATA& tag, \
                               WIZDOCUMENTDATA& newDoc);

    bool createDocumentByTemplate(const QString& templateZiwFile, \
                                  const QString& strLocation, \
                                  const WIZTAGDATA& tag, \
                                  WIZDOCUMENTDATA& newDoc);

    bool addAttachment(const WIZDOCUMENTDATA& document, \
                       const CString& strFileName, \
                       WIZDOCUMENTATTACHMENTDATA& dataRet);


    bool documentToTempHtmlFile(const WIZDOCUMENTDATA& document, \
                                QString& strFullPathFileName);
    bool documentToHtmlFile(const WIZDOCUMENTDATA& document, \
                            const QString& strPath);
    bool exportToHtmlFile(const WIZDOCUMENTDATA& document,
                          const QString& strIndexFileName);

    bool extractZiwFileToFolder(const WIZDOCUMENTDATA& document, const QString& strFolder);
    bool encryptDocument(WIZDOCUMENTDATA& document);
    bool compressFolderToZiwFile(WIZDOCUMENTDATA& document, const QString& strFileFolder);
    bool compressFolderToZiwFile(WIZDOCUMENTDATA& document, \
                                const QString& strFileFolder, const QString& strZiwFileName);
    bool cancelDocumentEncryption(WIZDOCUMENTDATA& document);

    bool isFileAccessible(const WIZDOCUMENTDATA& document);

    // CWizZiwReader passthrough methods
    bool loadUserCert();

    bool refreshCertFromServer();
    bool hasCert();
    bool initCert(bool queryPassword);

    bool isEncryptAllData();
    bool prepareBizCert();
    bool initBizCert();
    bool QueryCertPassword();
    bool verifyCertPassword(QString password);
    QString getCertPassword();
    QString getCertPasswordHint();

    static void clearCertPassword();


    bool tryAccessDocument(const WIZDOCUMENTDATA &doc);

public:
    Q_INVOKABLE QObject* GetDeletedItemsFolder();
    Q_INVOKABLE QObject* GetFolderByLocation(const QString& strLocation, bool bCreate);
    Q_INVOKABLE QStringList GetAllLocations();
    Q_INVOKABLE QStringList GetAllTags();
    Q_INVOKABLE QObject *DocumentFromGUID(const QString &strGUID);
    Q_INVOKABLE QVariantList DocumentsFromSQLWhere(const QString& strSQLWhere);
    Q_INVOKABLE QVariantList GetRecentDocuments(const QString &documentType, int count, int type);
    Q_INVOKABLE QObject *AttachmentFromGUID(const QString &attachmentGUID);
    Q_INVOKABLE bool SetMeta(const QString &section, const QString &key, const QString & value);
    Q_INVOKABLE QString GetMeta(const QString &section, const QString &key);

public slots:
    void onAttachmentModified(const QString strKbGUID, const QString& strGUID, const QString& strFileName,
                              const QString& strMD5, const QDateTime& dtLastModified);    

Q_SIGNALS:
    void userInfoChanged();
    void groupsInfoDownloaded(const CWizGroupDataArray& arrayGroup);
    void bizInfoDownloaded(const CWizBizDataArray& arrayBiz);
    void databaseOpened(WizDatabase* db, const QString& strKbGUID);
    void databaseRename(const QString& strKbGUID);
    void databasePermissionChanged(const QString& strKbGUID);
    void databaseBizChanged(const QString& strKbGUID);
    void updateError(const QString& msg);
    void processLog(const QString& msg);
    void attachmentsUpdated();
    void folderPositionChanged();
    void tagsPositionChanged(const QString& strKbGUID);
    void documentUploaded(const QString& strKbGUID, const QString& strGUID);
    void userIdChanged(const QString& oldId, const QString& newId);
    void favoritesChanged(const QString& favorites);

private:
    //should make sure sourceDoc already exist before use this.
    bool copyDocumentData(const WIZDOCUMENTDATA& sourceDoc, WizDatabase& targetDB, \
                                WIZDOCUMENTDATA& targetDoc);    

    bool getBizMetaName(const QString& strBizGUID, QString& strMetaName);

    bool initZiwReaderForEncryption();

    bool getAllGroupInfoCore(CWizGroupDataArray& arrayGroup);
    bool setAllGroupInfoCore(const CWizGroupDataArray& arrayGroup);

    bool getAllBizInfoCore(const CWizGroupDataArray& arrayGroup, CWizBizDataArray& arrayBiz);
    bool setAllBizInfoCore(const CWizBizDataArray& arrayBiz);
    QVariantList packDocumentsToList(const CWizDocumentDataArray &docDataArray);
    bool emptyProtectedAbstract(const QString &docGuid);
};


class WizDocumentDataLocker
{
    QMutex* m_mutex;

#ifdef QT_DEBUG
    QString m_docGuid;
#endif

public:
    WizDocumentDataLocker(QString docGuid);
    ~WizDocumentDataLocker();
};


#define WIZNOTE_MIMEFORMAT_TAGS             "wiznote/tags"
#define WIZNOTE_MIMEFORMAT_DOCUMENTS        "wiznote/documents"
#define WIZNOTE_MIMEFORMAT_FOLDER           "wiznote/folder"


#endif // WIZDATABASE_H
