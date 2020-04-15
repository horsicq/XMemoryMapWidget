// copyright (c) 2020 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef XMEMORYMAPWIDGET_H
#define XMEMORYMAPWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelection>
#include "xformats.h"
#include "xlineedithex.h"

namespace Ui {
class XMemoryMapWidget;
}

class XMemoryMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit XMemoryMapWidget(QWidget *parent=nullptr);
    ~XMemoryMapWidget();
    void setData(QIODevice *pDevice);

private slots:
    void on_comboBoxType_currentIndexChanged(int index);

    void on_radioButtonFileOffset_toggled(bool checked);
    void on_radioButtonVirtualAddress_toggled(bool checked);
    void on_radioButtonRelativeVirtualAddress_toggled(bool checked);
    void updateMemoryMap();
    void ajust(bool bInit);

    void on_lineEditFileOffset_textChanged(const QString &arg1);
    void on_lineEditVirtualAddress_textChanged(const QString &arg1);
    void on_lineEditRelativeVirtualAddress_textChanged(const QString &arg1);

    void on_tableViewSelection(const QItemSelection &selected, const QItemSelection &deselected);

    void _goToOffset(qint64 nOffset);

private:
    Ui::XMemoryMapWidget *ui;
    QIODevice *pDevice;
    XBinary::_MEMORY_MAP memoryMap;
    XLineEditHEX::MODE mode;
};

#endif // XMEMORYMAPWIDGET_H
