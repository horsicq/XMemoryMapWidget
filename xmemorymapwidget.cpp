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
#include "xmemorymapwidget.h"
#include "ui_xmemorymapwidget.h"

XMemoryMapWidget::XMemoryMapWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::XMemoryMapWidget)
{
    ui->setupUi(this);
}

XMemoryMapWidget::~XMemoryMapWidget()
{
    delete ui;
}

void XMemoryMapWidget::setData(QIODevice *pDevice)
{
    this->pDevice=pDevice;
    ui->widgetHex->setData(pDevice);

    const bool bWasBlocked=ui->comboBoxType->blockSignals(true);

    ui->comboBoxType->clear();

    QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(XBinary::getFileTypes(pDevice));

    int nCount=listFileTypes.count();

    for(int i=0;i<nCount;i++)
    {
        XBinary::FT ft=listFileTypes.at(i);
        ui->comboBoxType->addItem(XBinary::fileTypeIdToString(ft),ft);
    }

    ui->comboBoxType->blockSignals(bWasBlocked);

    if(nCount)
    {
        ui->comboBoxType->setCurrentIndex(nCount-1);
    }
}

void XMemoryMapWidget::process()
{

}

void XMemoryMapWidget::on_comboBoxType_currentIndexChanged(int index)
{
    XBinary::FT ft=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    memoryMap=XFormats::getMemoryMap(pDevice,ft);

    ui->radioButtonFileOffset->setChecked(true);

    ui->lineEditFileOffset->setValue((quint32)0);

    ajust();
}

void XMemoryMapWidget::on_radioButtonFileOffset_toggled(bool checked)
{
    Q_UNUSED(checked)
    ajust();
}

void XMemoryMapWidget::on_radioButtonVirtualAddress_toggled(bool checked)
{
    Q_UNUSED(checked)
    ajust();
}

void XMemoryMapWidget::on_radioButtonRelativeVirtualAddress_toggled(bool checked)
{
    Q_UNUSED(checked)
    ajust();
}

void XMemoryMapWidget::ajust()
{
    quint64 nOffset=ui->lineEditFileOffset->getValue();
    quint64 nVirtualAddress=ui->lineEditVirtualAddress->getValue();
    quint64 nRelativeVirtualAddress=ui->lineEditRelativeVirtualAddress->getValue();

    if(ui->radioButtonFileOffset->isChecked())
    {

    }
    else if(ui->radioButtonVirtualAddress->isChecked())
    {

    }
    else if(ui->radioButtonRelativeVirtualAddress->isChecked())
    {

    }
}

void XMemoryMapWidget::on_lineEditFileOffset_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ajust();
}

void XMemoryMapWidget::on_lineEditVirtualAddress_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ajust();
}

void XMemoryMapWidget::on_lineEditRelativeVirtualAddress_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ajust();
}
