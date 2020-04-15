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

    mode=XLineEditHEX::MODE_16;
}

XMemoryMapWidget::~XMemoryMapWidget()
{
    delete ui;
}

void XMemoryMapWidget::setData(QIODevice *pDevice)
{
    this->pDevice=pDevice;
    ui->widgetHex->setData(pDevice);

    const QSignalBlocker blocker(ui->comboBoxType);

    ui->comboBoxType->clear();

    QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(XBinary::getFileTypes(pDevice));

    int nCount=listFileTypes.count();

    for(int i=0;i<nCount;i++)
    {
        XBinary::FT ft=listFileTypes.at(i);
        ui->comboBoxType->addItem(XBinary::fileTypeIdToString(ft),ft);
    }

    if(nCount)
    {
        ui->comboBoxType->setCurrentIndex(nCount-1);
        updateMemoryMap();
    }
}

void XMemoryMapWidget::on_comboBoxType_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    updateMemoryMap();
}

void XMemoryMapWidget::on_radioButtonFileOffset_toggled(bool checked)
{
    Q_UNUSED(checked)
    ajust(false);
}

void XMemoryMapWidget::on_radioButtonVirtualAddress_toggled(bool checked)
{
    Q_UNUSED(checked)
    ajust(false);
}

void XMemoryMapWidget::on_radioButtonRelativeVirtualAddress_toggled(bool checked)
{
    Q_UNUSED(checked)
    ajust(false);
}

void XMemoryMapWidget::updateMemoryMap()
{
    const QSignalBlocker blocker1(ui->lineEditFileOffset);
    const QSignalBlocker blocker2(ui->lineEditVirtualAddress);
    const QSignalBlocker blocker3(ui->lineEditRelativeVirtualAddress);
    const QSignalBlocker blocker4(ui->tableViewMemoryMap);
    const QSignalBlocker blocker5(ui->pageHex);

    XBinary::FT ft=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    memoryMap=XFormats::getMemoryMap(pDevice,ft);

    ui->labelArch->setText(memoryMap.sArch);
    ui->labelMode->setText(XBinary::modeIdToString(memoryMap.mode));
    ui->labelEndianness->setText(XBinary::endiannessToString(memoryMap.bIsBigEndian));

    ui->radioButtonFileOffset->setChecked(true);

    ui->lineEditFileOffset->setValue((quint32)0);

    if(memoryMap.mode==XBinary::MODE_16)
    {
        mode=XLineEditHEX::MODE_16;
    }
    else if(memoryMap.mode==XBinary::MODE_32)
    {
        mode=XLineEditHEX::MODE_32;
    }
    else if(memoryMap.mode==XBinary::MODE_64)
    {
        mode=XLineEditHEX::MODE_64;
    }
    else if(memoryMap.mode==XBinary::MODE_UNKNOWN)
    {     
        mode=XLineEditHEX::getModeFromSize(memoryMap.nRawSize);
    }

    QAbstractItemModel *pOldModel=ui->tableViewMemoryMap->model();

    int nCount=memoryMap.listRecords.count();

    QStandardItemModel *pModel=new QStandardItemModel(nCount,4,this);

    pModel->setHeaderData(0,Qt::Horizontal,tr("Name"));
    pModel->setHeaderData(1,Qt::Horizontal,tr("Offset"));
    pModel->setHeaderData(2,Qt::Horizontal,tr("Address"));
    pModel->setHeaderData(3,Qt::Horizontal,tr("Size"));

    QColor colDisabled=QWidget::palette().color(QPalette::Window);

    for(int i=0;i<nCount;i++)
    {
        bool bIsVirtual=memoryMap.listRecords.at(i).bIsVirtual;

        QStandardItem *itemName=new QStandardItem;

        itemName->setData(memoryMap.listRecords.at(i).nOffset,Qt::UserRole+0);
        itemName->setData(memoryMap.listRecords.at(i).nAddress,Qt::UserRole+1);

        if(bIsVirtual)
        {
            itemName->setBackground(colDisabled);
        }

        itemName->setText(memoryMap.listRecords.at(i).sName);
        pModel->setItem(i,0,itemName);

        QStandardItem *itemOffset=new QStandardItem;

        if(bIsVirtual)
        {
            itemOffset->setBackground(colDisabled);
        }

        itemOffset->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nOffset));
        pModel->setItem(i,1,itemOffset);

        QStandardItem *itemAddress=new QStandardItem;

        if(bIsVirtual)
        {
            itemAddress->setBackground(colDisabled);
        }

        itemAddress->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nAddress));
        pModel->setItem(i,2,itemAddress);

        QStandardItem *itemSize=new QStandardItem;

        if(bIsVirtual)
        {
            itemSize->setBackground(colDisabled);
        }

        itemSize->setText(XLineEditHEX::getFormatString(mode,memoryMap.listRecords.at(i).nSize));
        pModel->setItem(i,3,itemSize);
    }

    ui->tableViewMemoryMap->setModel(pModel);

    delete pOldModel;

    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);

    qint32 nColumnSize=XLineEditHEX::getWidthFromMode(mode);

    ui->tableViewMemoryMap->setColumnWidth(1,nColumnSize);
    ui->tableViewMemoryMap->setColumnWidth(2,nColumnSize);
    ui->tableViewMemoryMap->setColumnWidth(3,nColumnSize);

    connect(ui->tableViewMemoryMap->selectionModel(),SIGNAL(selectionChanged(QItemSelection, QItemSelection)),this,SLOT(on_tableViewSelection(QItemSelection, QItemSelection)));

    ajust(true);
}

void XMemoryMapWidget::ajust(bool bInit)
{
    const QSignalBlocker blocker1(ui->lineEditFileOffset);
    const QSignalBlocker blocker2(ui->lineEditVirtualAddress);
    const QSignalBlocker blocker3(ui->lineEditRelativeVirtualAddress);
    const QSignalBlocker blocker4(ui->tableViewMemoryMap);
    const QSignalBlocker blocker5(ui->pageHex);

    int nTableViewIndex=-1;

    quint64 nFileOffset=ui->lineEditFileOffset->getValue();
    quint64 nVirtualAddress=ui->lineEditVirtualAddress->getValue();
    quint64 nRelativeVirtualAddress=ui->lineEditRelativeVirtualAddress->getValue();

    if(ui->radioButtonFileOffset->isChecked())
    {
        ui->lineEditFileOffset->setReadOnly(false);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nVirtualAddress=XBinary::offsetToAddress(&memoryMap,nFileOffset);
        nRelativeVirtualAddress=XBinary::offsetToRelAddress(&memoryMap,nFileOffset);

        XBinary::_MEMORY_RECORD memoryRecord=XBinary::getMemoryRecordByOffset(&memoryMap,nFileOffset);

        if(memoryRecord.nSize)
        {
            nTableViewIndex=memoryRecord.nIndex;
        }

        if(bInit)
        {
            ui->lineEditFileOffset->setModeValue(mode,nFileOffset);
        }

        ui->lineEditVirtualAddress->setModeValue(mode,nVirtualAddress);
        ui->lineEditRelativeVirtualAddress->setModeValue(mode,nRelativeVirtualAddress);
    }
    else if(ui->radioButtonVirtualAddress->isChecked())
    {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(false);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nFileOffset=XBinary::addressToOffset(&memoryMap,nVirtualAddress);
        nRelativeVirtualAddress=XBinary::addressToRelAddress(&memoryMap,nVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord=XBinary::getMemoryRecordByAddress(&memoryMap,nVirtualAddress);

        if(memoryRecord.nSize)
        {
            nTableViewIndex=memoryRecord.nIndex;
        }

        if(bInit)
        {
            ui->lineEditVirtualAddress->setModeValue(mode,nVirtualAddress);
        }

        ui->lineEditFileOffset->setModeValue(mode,nFileOffset);
        ui->lineEditRelativeVirtualAddress->setModeValue(mode,nRelativeVirtualAddress);
    }
    else if(ui->radioButtonRelativeVirtualAddress->isChecked())
    {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(false);

        nFileOffset=XBinary::relAddressToOffset(&memoryMap,nRelativeVirtualAddress);
        nVirtualAddress=XBinary::relAddressToAddress(&memoryMap,nRelativeVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord=XBinary::getMemoryRecordByRelAddress(&memoryMap,nRelativeVirtualAddress);

        if(memoryRecord.nSize)
        {
            nTableViewIndex=memoryRecord.nIndex;
        }

        if(bInit)
        {
            ui->lineEditRelativeVirtualAddress->setModeValue(mode,nRelativeVirtualAddress);
        }

        ui->lineEditFileOffset->setModeValue(mode,nFileOffset);
        ui->lineEditVirtualAddress->setModeValue(mode,nVirtualAddress);
    }

    if(nTableViewIndex!=-1)
    {
        ui->tableViewMemoryMap->setCurrentIndex(ui->tableViewMemoryMap->model()->index(nTableViewIndex,0));
    }

    _goToOffset(nFileOffset);
}

void XMemoryMapWidget::on_lineEditFileOffset_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ajust(false);
}

void XMemoryMapWidget::on_lineEditVirtualAddress_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ajust(false);
}

void XMemoryMapWidget::on_lineEditRelativeVirtualAddress_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    ajust(false);
}

void XMemoryMapWidget::on_tableViewSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    const QSignalBlocker blocker1(ui->lineEditFileOffset);
    const QSignalBlocker blocker2(ui->lineEditVirtualAddress);
    const QSignalBlocker blocker3(ui->lineEditRelativeVirtualAddress);
    const QSignalBlocker blocker4(ui->tableViewMemoryMap);
    const QSignalBlocker blocker5(ui->pageHex);

    QItemSelectionModel *pSelectionModel=ui->tableViewMemoryMap->selectionModel();

    if(pSelectionModel)
    {
        QModelIndexList listIndexes=pSelectionModel->selectedRows(0);

        if(listIndexes.count())
        {
            qint64 nFileOffset=listIndexes.at(0).data(Qt::UserRole+0).toLongLong();
            qint64 nVirtualAddress=listIndexes.at(0).data(Qt::UserRole+1).toLongLong();
            qint64 nRelativeVirtualAddress=XBinary::addressToRelAddress(&memoryMap,nVirtualAddress);

            ui->lineEditFileOffset->setModeValue(mode,nFileOffset);
            ui->lineEditVirtualAddress->setModeValue(mode,nVirtualAddress);
            ui->lineEditRelativeVirtualAddress->setModeValue(mode,nRelativeVirtualAddress);

            _goToOffset(nFileOffset);
        }
    }

}

void XMemoryMapWidget::_goToOffset(qint64 nOffset)
{
    if(XBinary::isOffsetValid(&memoryMap,nOffset))
    {
        ui->stackedWidgetHex->setCurrentIndex(0);

        ui->widgetHex->goToOffset(nOffset);
        ui->widgetHex->reload();
    }
    else
    {
        // Invalid offset
        ui->stackedWidgetHex->setCurrentIndex(1);
    }
}
