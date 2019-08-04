var objApp = null;
var objDatabase = null;

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
    //
    var year = parseInt(a[0], 10);
    var month = parseInt(a[1], 10);
    var day = parseInt(a[2], 10);
    //
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
//
async function listDocuments() {
    const documents = await objDatabase.GetRecentDocuments("", 10);
    for (const doc of documents) {
        const item = document.createElement("li");
        const date = new Date(doc.DateModified);
        const dateString = date.toLocaleDateString();
        // Update UI
        item.innerHTML = "<a href=\"javascript:void(0);\" onclick=\"viewDocument('" + doc.GUID + "');\" >" + doc.Title + "</a>&nbsp;(" + dateString + ")";
        listRecent.appendChild(item);
        // Free up memory space of C++ Object
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
    const and1 = ` and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME = 'CALENDAR_START'  and  PARAM_VALUE <= '${dateToStr(end) + " 23:59:59"}' )`;
    const and2 = ` and DOCUMENT_GUID in (select DOCUMENT_GUID from WIZ_DOCUMENT_PARAM where PARAM_NAME = 'CALENDAR_END'  and  PARAM_VALUE >= '${dateToStr(begin) + " 00:00:00"}' )`;
    sql += and2;
    sql += and1;
    // Get documents which contains events data
    const events = await objDatabase.DocumentsFromSQLWhere(sql);
    if (events == undefined || events.length == 0) {
        const item = document.createElement("li");
        item.innerHTML = "No events";
        listTodo.appendChild(item);
    } else {
        for (const doc of events) {
            var item = document.createElement("li");
            // Get start date of event
            const eventBegin = strToDate(await doc.GetParamValue("CALENDAR_START"));
            const eventBeginString = eventBegin.toLocaleDateString();
            //
            item.innerHTML = "<a href=\"javascript:void(0);\" onclick=\"viewDocument('" + doc.GUID + "');\" >" + doc.Title + "</a>&nbsp;(" + eventBeginString + ")";
            listTodo.appendChild(item);
            //
            doc.deleteLater();
        }
    }
}
//
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
    //
    var sql = "DT_CREATED >='" + begin_date_string + "' and DT_CREATED <='" + end_date_string + "'"; //  and DOCUMENT_TYPE='journal'";
    //
    var documents = await objDatabase.DocumentsFromSQLWhere(sql);
    //
    if (documents == null)
        return 0;
    //
    objApp.DocumentsCtrl.SetDocuments(documents);
    //
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

/*TODO: Localize document
var languageFileName = null;

function getLanguageFileName(objApp) {
    if (languageFileName == null || languageFileName == "") {
        var path = objApp.GetHtmlDocumentPath(document);
        languageFileName = path + "welcome.ini";
    }
    //
    return languageFileName;
}
//TODO
function localizeCurrentDocument(objApp) {
    objApp.LocalizeHtmlDocument(getLanguageFileName(objApp), document);
}
//
localizeCurrentDocument(objApp);
*/

$(document).ready(function(){
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
        // 
        await listDocuments();
        await listEvents();
        // Display page
        $('body').css('visibility', 'visible');
    });
})