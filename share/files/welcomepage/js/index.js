var objApp = null;
var objDatabase = null;

const ITEM_LIMITS = 10;

const en_US = {
    "labelOverview": "Overview",
    "labelNews": "News",
    "labelRecentDocuments": "Recent Documents",
    "labelRecentAccessedDocuments": "Recent Read",
    "labelUnreadDocuments": "Unread Documents",
    "labelTodo": "Todo",
    "Title": "Welcome to Wiz"
}

const zh_CN = {
    "labelOverview": "近期概况",
    "labelNews": "为知笔记动态",
    "labelRecentDocuments": "近期文档",
    "labelRecentAccessedDocuments": "近期阅读",
    "labelUnreadDocuments": "未读文档",
    "labelTodo": "待办事宜",
    "Title": "欢迎使用为知笔记"
}

const zh_TW = {
    "labelOverview": "近期概況",
    "labelNews": "為知動態",
    "labelRecentDocuments": "近期筆記",
    "labelRecentAccessedDocuments": "近期閲讀",
    "labelUnreadDocuments": "未讀筆記",
    "labelTodo": "待辦事項",
    "Title": "歡迎使用為知筆記"
}

function dateToStr(dt) {
    return "" + dt.getFullYear() + "-" + (dt.getMonth() + 1) + "-" + dt.getDate();
}

function strToDate(str) {
    var timePos = str.indexOf(" ");
    if (timePos != -1) {
        str = str.substring(0, timePos);
    }
    var a = str.split('-');
    if (a.length != 3)
        return null;

    var year = parseInt(a[0], 10);
    var month = parseInt(a[1], 10);
    var day = parseInt(a[2], 10);

    var ret = new Date();
    ret.setFullYear(year, month - 1, day);
    return ret;
}

async function viewDocument(doc_guid) {
    const doc = await objDatabase.DocumentFromGUID(doc_guid);
    if (doc != null) {
        objApp.Window.ViewDocument(doc, true);
    }
    doc.deleteLater();
}

function getMetaInt(objDatabase, metaName, metaKey, defVal) {
    var value = objDatabase.Meta(metaName, metaKey);
    if (value == null)
        return defVal;
    if (value == "")
        return defVal;
    try {
        return parseInt(value, 10);
    }
    catch (e) {
        return defVal;
    }
}

async function listDocuments() {
    const documents = await objDatabase.GetRecentDocuments("", ITEM_LIMITS, 0);
    listRecent.innerHTML = "";
    for (const doc of documents) {
        const item = document.createElement("li");
        const docDate = doc.DateModified == "1970-01-01T08:00:00" ? doc.DateCreated : doc.DateModified;
        const date = new Date(docDate);
        const dateString = date.toLocaleDateString();
        // Update UI
        item.innerHTML = "<a href=\"javascript:void(0);\" onclick=\"viewDocument('" + doc.GUID + "');\" >" + doc.Title + "</a>&nbsp;(" + dateString + ")";
        listRecent.appendChild(item);
        // Free up memory space of C++ Object
        doc.deleteLater();
    }
}

async function listDocumentsRecentAccessed() {
    const documents = await objDatabase.GetRecentDocuments("", ITEM_LIMITS, 3);
    listRecentAccessed.innerHTML = "";
    for (const doc of documents) {
        const item = document.createElement("li");
        const docDate = doc.DateModified == "1970-01-01T08:00:00" ? doc.DateCreated : doc.DateModified;
        const date = new Date(docDate);
        const dateString = date.toLocaleDateString();
        // Update UI
        item.innerHTML = "<a href=\"javascript:void(0);\" onclick=\"viewDocument('" + doc.GUID + "');\" >" + doc.Title + "</a>&nbsp;(" + dateString + ")";
        listRecentAccessed.appendChild(item);
        // Free up memory space of C++ Object
        doc.deleteLater();
    }
}

async function listUnreadDocuments() {
    let sql = `DOCUMENT_LOCATION not like '/Deleted Items/%'` + 
        // select zero read count documents
        ` and DOCUMENT_READ_COUNT=0` + 
        // Do not select calendar events
        ` and DOCUMENT_GUID not in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME = 'CALENDAR_START')` +
        // Random select
        ` order by random() limit ${ITEM_LIMITS}`;

    const documents = await objDatabase.DocumentsFromSQLWhere(sql);
    listUnread.innerHTML = "";
    if (documents.length == 0) {
        const item = document.createElement("li");
        item.innerHTML = "No documents";
        listUnread.appendChild(item);
        return;
    }
    for (const doc of documents) {
        const item = document.createElement("li");
        const docDate = doc.DateModified == "1970-01-01T08:00:00" ? doc.DateCreated : doc.DateModified;
        const date = new Date(docDate);
        const dateString = date.toLocaleDateString();
        //
        item.innerHTML = "<a href=\"javascript:void(0);\" onclick=\"viewDocument('" + doc.GUID + "');\" >" + doc.Title + "</a>&nbsp;(" + dateString + ")";
        listUnread.appendChild(item);
        //
        doc.deleteLater();
    }
}

async function listEvents() {
    // Get query data
    const begin = new Date();
    const end = new Date();
    end.setDate(end.getDate() + 7);
    // Construct SQL
    let sql = `DOCUMENT_LOCATION not like '/Deleted Items/%'`;
    const and1 = ` and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME = 'CALENDAR_START'  and  PARAM_VALUE <= '${end.toISOString().replace(/T.*/, "") + " 23:59:59"}' )`;
    const and2 = ` and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME = 'CALENDAR_END'  and  PARAM_VALUE >= '${begin.toISOString().replace(/T.*/, "") + " 00:00:00"}' )`;
    sql += and2;
    sql += and1;
    // Get documents which contains events data
    // Wait for event creation
    await new Promise(r => setTimeout(r, 1000));
    const events = await objDatabase.DocumentsFromSQLWhere(sql);
    let liItems = [];
    if (events == undefined || events.length == 0) {
        const item = document.createElement("li");
        item.innerHTML = "No events";
        liItems.push(item)
    } else {
        for (const doc of events) {
            // Get start date of event
            const eventBegin = strToDate(await doc.GetParamValue("CALENDAR_START"));
            const eventBeginString = eventBegin.toLocaleDateString();

            const item = document.createElement("li");
            item.innerHTML = "<a href=\"javascript:void(0);\" onclick=\"viewDocument('" + doc.GUID + "');\" >" + doc.Title + "</a>&nbsp;(" + eventBeginString + ")";
            liItems.push(item)

            doc.deleteLater();
        }
    }

    listTodo.innerHTML = "";
    for (const liItem of liItems) {
        listTodo.appendChild(liItem);
    }
}

function formatInt(val) {
    if (val < 10)
        return "0" + val;
    else
        return "" + val;
}

async function listDocumentsByDate(dt) {
    if (typeof (objApp) == "undefined")
        return 0;

    var begin_date_string = dt.getFullYear() + "-" + formatInt(dt.getMonth() + 1) + "-" + formatInt(dt.getDate()) + " 00:00:00";
    var end_date_string = dt.getFullYear() + "-" + formatInt(dt.getMonth() + 1) + "-" + formatInt(dt.getDate()) + " 23:59:59";

    var sql = "DT_CREATED >='" + begin_date_string + "' and DT_CREATED <='" + end_date_string + "'"; //  and DOCUMENT_TYPE='journal'";

    var documents = await objDatabase.DocumentsFromSQLWhere(sql);

    if (documents == null)
        return 0;

    const guidList = new Array();
    for (const doc of documents) {
        guidList.push(doc.GUID);
        doc.deleteLater();
    }

    objApp.DocumentsCtrl.SetDocuments(guidList);

    return 0;
}

function viewDocumentsByDate(dateText) {
    var date = strToDate(dateText);
    try {
        listDocumentsByDate(date);
    }
    catch (err) {
    }
}

$(document).ready(function() {
    // Initialize UI
    $('#tabs').tabs();
    $('#datepicker').datepicker({
        inline: true,
        dateFormat: 'yy-mm-dd',
        onSelect: function(date) { viewDocumentsByDate(date); },
        prevText: '',
        nextText: ''
    });
    $('#dialog_link, ul#icons li').hover(
        function() { $(this).addClass('ui-state-hover'); },
        function() { $(this).removeClass('ui-state-hover'); }
    );

    // Initialize WizNote APIs
    new QWebChannel(qt.webChannelTransport, async function(channel) {
        var objectNames = ["WizExplorerApp"];
        for (var i = 0; i < objectNames.length; i++) {
            var key = objectNames[i];
            window[key] = channel.objects[key];
        }
        // Get APIs
        objApp = WizExplorerApp;
        objDatabase = WizExplorerApp.Database;
        // Localize html
        const lang = await objApp.Locale();
        const t = await i18next.init({
            lng: lang,
            resources: {
                "en_US": {
                    translation: en_US
                },
                "zh_CN": {
                    translation: zh_CN
                },
                "zh_TW": {
                    translation: zh_TW
                }
            }
        });
        for (const id of Object.keys(en_US)) {
            if (id == "Title") {
                document.title = t("Title");
                continue;
            }

            const elem = document.getElementById(id);
            if (elem)
                elem.innerHTML = t(id);
        }
        $('#datepicker').datepicker(
            $.datepicker.regional[lang.replace(/_/, "-")]
        );

        objDatabase.documentCreated.connect(listDocuments);
        objDatabase.documentDeleted.connect(listDocuments);
        objDatabase.documentDeleted.connect(listEvents);
        objDatabase.documentModified.connect(listDocuments);
        objDatabase.documentModified.connect(listEvents);
        objDatabase.documentDataModified.connect(listDocuments);
        objDatabase.documentAccessDateModified.connect(listDocumentsRecentAccessed);

        await listDocuments();
        await listDocumentsRecentAccessed();
        await listEvents();
        await listUnreadDocuments();
        // Display page
        $('body').css('visibility', 'visible');
    });
})
