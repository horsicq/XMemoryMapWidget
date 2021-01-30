// copyright (c) 2020-2021 hors<horsicq@gmail.com>
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

XMemoryMapWidget::XMemoryMapWidget(QWidget *pParent) :
    QWidget(pParent),
    ui(new Ui::XMemoryMapWidget)
{
    ui->setupUi(this);

    g_mode=XLineEditHEX::MODE_16;
}

XMemoryMapWidget::~XMemoryMapWidget()
{
    delete ui;
}

void XMemoryMapWidget::setData(QIODevice *pDevice, XBinary::FT fileType)
{
    this->g_pDevice=pDevice;
    ui->widgetHex->setData(pDevice);

    QSet<XBinary::FT> stFileType=XBinary::getFileTypes(pDevice,true);

    stFileType.insert(XBinary::FT_COM);

    QList<XBinary::FT> listFileTypes=XBinary::_getFileTypeListFromSet(stFileType);

    XFormats::setFileTypeComboBox(ui->comboBoxType,&listFileTypes,fileType);

    updateMemoryMap();
}

void XMemoryMapWidget::on_comboBoxType_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    updateMemoryMap();
}

void XMemoryMapWidget::on_radioButtonFileOffset_toggled(bool bChecked)
{
    Q_UNUSED(bChecked)

    ajust(false);
}

void XMemoryMapWidget::on_radioButtonVirtualAddress_toggled(bool bChecked)
{
    Q_UNUSED(bChecked)

    ajust(false);
}

void XMemoryMapWidget::on_radioButtonRelativeVirtualAddress_toggled(bool bChecked)
{
    Q_UNUSED(bChecked)

    ajust(false);
}

void XMemoryMapWidget::updateMemoryMap()
{
    const QSignalBlocker blocker1(ui->lineEditFileOffset);
    const QSignalBlocker blocker2(ui->lineEditVirtualAddress);
    const QSignalBlocker blocker3(ui->lineEditRelativeVirtualAddress);
    const QSignalBlocker blocker4(ui->tableViewMemoryMap);
    const QSignalBlocker blocker5(ui->pageHex);

    XBinary::FT fileType=(XBinary::FT)(ui->comboBoxType->currentData().toInt());

    g_memoryMap=XFormats::getMemoryMap(fileType,g_pDevice);

    ui->lineEditArch->setText(g_memoryMap.sArch);
    ui->lineEditMode->setText(XBinary::modeIdToString(g_memoryMap.mode));
    ui->lineEditEndianness->setText(XBinary::endiannessToString(g_memoryMap.bIsBigEndian));

    ui->radioButtonFileOffset->setChecked(true);

    ui->lineEditFileOffset->setValue((quint32)0);

    if(g_memoryMap.mode==XBinary::MODE_16)
    {
        g_mode=XLineEditHEX::MODE_16;
    }
    else if((g_memoryMap.mode==XBinary::MODE_16SEG)||(g_memoryMap.mode==XBinary::MODE_32)||(g_memoryMap.mode==XBinary::MODE_DATA)) // TODO Check Data
    {
        g_mode=XLineEditHEX::MODE_32;
    }
    else if(g_memoryMap.mode==XBinary::MODE_64)
    {
        g_mode=XLineEditHEX::MODE_64;
    }
    else if(g_memoryMap.mode==XBinary::MODE_UNKNOWN)
    {     
        g_mode=XLineEditHEX::getModeFromSize(g_memoryMap.nRawSize);
    }

    QAbstractItemModel *pOldModel=ui->tableViewMemoryMap->model();

    int nNumberOfRecords=g_memoryMap.listRecords.count();

    QStandardItemModel *pModel=new QStandardItemModel(nNumberOfRecords,4,this);

    pModel->setHeaderData(0,Qt::Horizontal,tr("Name"));
    pModel->setHeaderData(1,Qt::Horizontal,tr("Offset"));
    pModel->setHeaderData(2,Qt::Horizontal,tr("Address"));
    pModel->setHeaderData(3,Qt::Horizontal,tr("Size"));

//    QColor colDisabled=QWidget::palette().color(QPalette::Window);

    for(int i=0;i<nNumberOfRecords;i++)
    {
        bool bIsVirtual=g_memoryMap.listRecords.at(i).bIsVirtual;

        QStandardItem *pItemName=new QStandardItem;

        pItemName->setData(g_memoryMap.listRecords.at(i).nOffset,Qt::UserRole+0);
        pItemName->setData(g_memoryMap.listRecords.at(i).nAddress,Qt::UserRole+1);
        pItemName->setData(g_memoryMap.listRecords.at(i).nSize,Qt::UserRole+2);

//        if(bIsVirtual)
//        {
//            pItemName->setBackground(colDisabled);
//        }

        pItemName->setText(g_memoryMap.listRecords.at(i).sName);
        pModel->setItem(i,0,pItemName);

        QStandardItem *pItemOffset=new QStandardItem;

//        if(bIsVirtual)
//        {
//            pItemOffset->setBackground(colDisabled);
//        }

        pItemOffset->setText(XLineEditHEX::getFormatString(g_mode,g_memoryMap.listRecords.at(i).nOffset));
        pModel->setItem(i,1,pItemOffset);

        QStandardItem *pItemAddress=new QStandardItem;

//        if(bIsVirtual)
//        {
//            pItemAddress->setBackground(colDisabled);
//        }

        pItemAddress->setText(XLineEditHEX::getFormatString(g_mode,g_memoryMap.listRecords.at(i).nAddress));
        pModel->setItem(i,2,pItemAddress);

        QStandardItem *pItemSize=new QStandardItem;

//        if(bIsVirtual)
//        {
//            pItemSize->setBackground(colDisabled);
//        }

        pItemSize->setText(XLineEditHEX::getFormatString(g_mode,g_memoryMap.listRecords.at(i).nSize));
        pModel->setItem(i,3,pItemSize);
    }

    ui->tableViewMemoryMap->setModel(pModel);

    delete pOldModel;

    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);

    qint32 nColumnSize=XLineEditHEX::getWidthFromMode(this,g_mode);

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

        nVirtualAddress=XBinary::offsetToAddress(&g_memoryMap,nFileOffset);
        nRelativeVirtualAddress=XBinary::offsetToRelAddress(&g_memoryMap,nFileOffset);

        XBinary::_MEMORY_RECORD memoryRecord=XBinary::getMemoryRecordByOffset(&g_memoryMap,nFileOffset);

        if(memoryRecord.nSize)
        {
            nTableViewIndex=memoryRecord.nIndex;
        }

        if(bInit)
        {
            ui->lineEditFileOffset->setModeValue(g_mode,nFileOffset);
        }

        ui->lineEditVirtualAddress->setModeValue(g_mode,nVirtualAddress);
        ui->lineEditRelativeVirtualAddress->setModeValue(g_mode,nRelativeVirtualAddress);
    }
    else if(ui->radioButtonVirtualAddress->isChecked())
    {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(false);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nFileOffset=XBinary::addressToOffset(&g_memoryMap,nVirtualAddress);
        nRelativeVirtualAddress=XBinary::addressToRelAddress(&g_memoryMap,nVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord=XBinary::getMemoryRecordByAddress(&g_memoryMap,nVirtualAddress);

        if(memoryRecord.nSize)
        {
            nTableViewIndex=memoryRecord.nIndex;
        }

        if(bInit)
        {
            ui->lineEditVirtualAddress->setModeValue(g_mode,nVirtualAddress);
        }

        ui->lineEditFileOffset->setModeValue(g_mode,nFileOffset);
        ui->lineEditRelativeVirtualAddress->setModeValue(g_mode,nRelativeVirtualAddress);
    }
    else if(ui->radioButtonRelativeVirtualAddress->isChecked())
    {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(false);

        nFileOffset=XBinary::relAddressToOffset(&g_memoryMap,nRelativeVirtualAddress);
        nVirtualAddress=XBinary::relAddressToAddress(&g_memoryMap,nRelativeVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord=XBinary::getMemoryRecordByRelAddress(&g_memoryMap,nRelativeVirtualAddress);

        if(memoryRecord.nSize)
        {
            nTableViewIndex=memoryRecord.nIndex;
        }

        if(bInit)
        {
            ui->lineEditRelativeVirtualAddress->setModeValue(g_mode,nRelativeVirtualAddress);
        }

        ui->lineEditFileOffset->setModeValue(g_mode,nFileOffset);
        ui->lineEditVirtualAddress->setModeValue(g_mode,nVirtualAddress);
    }

    if(nTableViewIndex!=-1)
    {
        ui->tableViewMemoryMap->setCurrentIndex(ui->tableViewMemoryMap->model()->index(nTableViewIndex,0));
    }

    _goToOffset(nFileOffset,1);
}

void XMemoryMapWidget::on_lineEditFileOffset_textChanged(const QString &sText)
{
    Q_UNUSED(sText)

    ajust(false);
}

void XMemoryMapWidget::on_lineEditVirtualAddress_textChanged(const QString &sText)
{
    Q_UNUSED(sText)

    ajust(false);
}

void XMemoryMapWidget::on_lineEditRelativeVirtualAddress_textChanged(const QString &sText)
{
    Q_UNUSED(sText)

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
            qint64 nSize=listIndexes.at(0).data(Qt::UserRole+2).toLongLong();

            qint64 nRelativeVirtualAddress=XBinary::addressToRelAddress(&g_memoryMap,nVirtualAddress);

            ui->lineEditFileOffset->setModeValue(g_mode,nFileOffset);
            ui->lineEditVirtualAddress->setModeValue(g_mode,nVirtualAddress);
            ui->lineEditRelativeVirtualAddress->setModeValue(g_mode,nRelativeVirtualAddress);

            _goToOffset(nFileOffset,nSize);
        }
    }

}

void XMemoryMapWidget::_goToOffset(qint64 nOffset, qint64 nSize)
{
    if(nSize==0)
    {
        nSize=1;
    }

    if(XBinary::isOffsetValid(&g_memoryMap,nOffset))
    {
        ui->stackedWidgetHex->setCurrentIndex(0);

        ui->widgetHex->goToOffset(nOffset);
        ui->widgetHex->setSelection(nOffset,nSize);
        ui->widgetHex->reload();
    }
    else
    {
        // Invalid offset
        ui->stackedWidgetHex->setCurrentIndex(1);
    }
}
