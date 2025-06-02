/* Copyright (c) 2020-2025 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XMEMORYMAPWIDGET_H
#define XMEMORYMAPWIDGET_H

#include <QItemSelection>
#include <QStandardItemModel>

#include "xformats.h"
#include "xlineedithex.h"
#include "xshortcutswidget.h"
#include "xinfodb.h"

namespace Ui {
class XMemoryMapWidget;
}

class XMemoryMapWidget : public XShortcutsWidget {
    Q_OBJECT

public:
    struct OPTIONS {
        XBinary::FT fileType;
        bool bIsSearchEnable;
    };

    explicit XMemoryMapWidget(QWidget *pParent = nullptr);
    ~XMemoryMapWidget();

    void setData(QIODevice *pDevice, const OPTIONS &options, XInfoDB *pXInfoDB);
    void setXInfoDB(XInfoDB *pXInfoDB);  // TODO remove
    void goToOffset(qint64 nOffset);     // TODO remove use setLocation
    virtual void setLocation(quint64 nLocation, qint32 nLocationType, qint64 nSize);
    void setGlobal(XShortcuts *pShortcuts, XOptions *pXOptions);
    virtual void adjustView();
    virtual void reloadData(bool bSaveSelection);

private slots:
    void on_comboBoxType_currentIndexChanged(int nIndex);
    void on_radioButtonFileOffset_toggled(bool bChecked);
    void on_radioButtonVirtualAddress_toggled(bool bChecked);
    void on_radioButtonRelativeVirtualAddress_toggled(bool bChecked);
    void updateMemoryMap();
    void _adjust(bool bInit);
    void on_lineEditFileOffset_textChanged(const QString &sText);
    void on_lineEditVirtualAddress_textChanged(const QString &sText);
    void on_lineEditRelativeVirtualAddress_textChanged(const QString &sText);
    void on_tableViewSelection(const QItemSelection &itemSelected, const QItemSelection &itemDeselected);
    void _goToOffset(qint64 nOffset, qint64 nSize = 0);
    void onHexCursorChanged(qint64 nOffset);
    void on_toolButtonSave_clicked();
    void on_checkBoxShowAll_stateChanged(int nValue);
    void on_toolButtonDumpAll_clicked();
    void on_tableViewMemoryMap_customContextMenuRequested(const QPoint &pos);
    void dumpSection();
    void on_tableViewMemoryMap_clicked(const QModelIndex &index);
    void viewSelection();
    void on_toolButtonFileOffsetFind_clicked();
    void on_toolButtonVirtualAddressFind_clicked();
    void on_toolButtonRelativeVirtualAddressFind_clicked();
    void on_comboBoxMapMode_currentIndexChanged(int nIndex);
    void on_toolButtonReload_clicked();

protected:
    virtual void registerShortcuts(bool bState);

signals:
    void findValue(quint64 nValue, XBinary::ENDIAN endian);

private:
    Ui::XMemoryMapWidget *ui;
    QIODevice *g_pDevice;
    OPTIONS g_options;
    XBinary::_MEMORY_MAP g_memoryMap;
    XLineEditValidator::MODE g_mode;
    bool g_bLockHex;
    QMap<qint32, qint32> g_mapIndexes;
    XInfoDB *g_pXInfoDB;
};

#endif  // XMEMORYMAPWIDGET_H
