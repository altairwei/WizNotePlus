#ifndef GUI_DOCLISTVIEWER_WIZDOCUMENTLISTVIEWSECTIONITEM_H
#define GUI_DOCLISTVIEWER_WIZDOCUMENTLISTVIEWSECTIONITEM_H

#include "WizDocumentListViewBaseItem.h"

class WizDocumentListViewDocumentItem;

class WizDocumentListViewSectionItem : public WizDocumentListViewBaseItem
{
    Q_OBJECT
public:
    explicit WizDocumentListViewSectionItem(const WizDocumentListViewSectionData& data, const QString& text, int docCount);
    const WizDocumentListViewSectionData& sectionData() const { return m_data; }

    // used for sorting
    virtual bool operator<(const QListWidgetItem &other) const;

    virtual void draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const;

private:
    bool compareWithDocumentItem(const WizDocumentListViewDocumentItem* docItem) const;

private:
    WizDocumentListViewSectionData m_data;
    QString m_text;
    int m_documentCount;
};

#endif // GUI_DOCLISTVIEWER_WIZDOCUMENTLISTVIEWSECTIONITEM_H