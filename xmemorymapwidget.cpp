/* Copyright (c) 2020-2023 hors<horsicq@gmail.com>
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
#include "xmemorymapwidget.h"

#include "ui_xmemorymapwidget.h"

XMemoryMapWidget::XMemoryMapWidget(QWidget *pParent) : XShortcutsWidget(pParent), ui(new Ui::XMemoryMapWidget)
{
    ui->setupUi(this);

    g_pDevice = nullptr;
    g_options = {};
    g_mode = XLineEditHEX::MODE_16;
    g_bLockHex = false;
    g_memoryMap = {};
    g_pXInfoDB = nullptr;
}

XMemoryMapWidget::~XMemoryMapWidget()
{
    delete ui;
}

void XMemoryMapWidget::setData(QIODevice *pDevice, OPTIONS options, XInfoDB *pXInfoDB)
{
    g_pDevice = pDevice;
    g_options = options;
    g_pXInfoDB = pXInfoDB;

    XHexView::OPTIONS hex_options = {};  // TODO Check !!!

    ui->widgetHex->setXInfoDB(pXInfoDB);
    ui->widgetHex->setData(pDevice, hex_options, true);

    if (pDevice) {
        XFormats::setFileTypeComboBox(options.fileType, g_pDevice, ui->comboBoxType);

        updateMemoryMap();
    }

    if (options.bIsSearchEnable) {
        ui->pushButtonFileOffsetFind->show();
        ui->pushButtonRelativeVirtualAddressFind->show();
        ui->pushButtonVirtualAddressFind->show();
    } else {
        ui->pushButtonFileOffsetFind->hide();
        ui->pushButtonRelativeVirtualAddressFind->hide();
        ui->pushButtonVirtualAddressFind->hide();
    }
}

void XMemoryMapWidget::setXInfoDB(XInfoDB *pXInfoDB)
{
    ui->widgetHex->setXInfoDB(pXInfoDB);
}

void XMemoryMapWidget::goToOffset(qint64 nOffset)
{
    ui->lineEditFileOffset->setModeValue(g_mode, nOffset);
}

void XMemoryMapWidget::setGlobal(XShortcuts *pShortcuts, XOptions *pXOptions)
{
    ui->widgetHex->setGlobal(pShortcuts, pXOptions);
    XShortcutsWidget::setGlobal(pShortcuts, pXOptions);
}

void XMemoryMapWidget::on_comboBoxType_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    updateMemoryMap();
}

void XMemoryMapWidget::on_radioButtonFileOffset_toggled(bool bChecked)
{
    Q_UNUSED(bChecked)

    _adjust(false);
}

void XMemoryMapWidget::on_radioButtonVirtualAddress_toggled(bool bChecked)
{
    Q_UNUSED(bChecked)

    _adjust(false);
}

void XMemoryMapWidget::on_radioButtonRelativeVirtualAddress_toggled(bool bChecked)
{
    Q_UNUSED(bChecked)

    _adjust(false);
}

void XMemoryMapWidget::updateMemoryMap()
{
    const bool bBlocked1 = ui->lineEditFileOffset->blockSignals(true);
    const bool bBlocked2 = ui->lineEditVirtualAddress->blockSignals(true);
    const bool bBlocked3 = ui->lineEditRelativeVirtualAddress->blockSignals(true);
    const bool bBlocked4 = ui->tableViewMemoryMap->blockSignals(true);
    const bool bBlocked5 = ui->pageHex->blockSignals(true);

    g_mapIndexes.clear();

    XBinary::FT fileType = (XBinary::FT)(ui->comboBoxType->currentData().toInt());

    g_memoryMap = XFormats::getMemoryMap(fileType, g_pDevice);

    ui->lineEditArch->setText(g_memoryMap.sArch);
    ui->lineEditMode->setText(XBinary::modeIdToString(g_memoryMap.mode));
    ui->lineEditEndianness->setText(XBinary::endiannessToString(g_memoryMap.bIsBigEndian));

    ui->radioButtonFileOffset->setChecked(true);

    ui->lineEditFileOffset->setValue((quint32)0);

    XBinary::MODE _mode = XBinary::getWidthModeFromMemoryMap(&g_memoryMap);

    // TODO move function to XShortcutWidget !!!
    if (_mode == XBinary::MODE_8) g_mode = XLineEditHEX::MODE_8;
    else if (_mode == XBinary::MODE_16) g_mode = XLineEditHEX::MODE_16;
    else if (_mode == XBinary::MODE_32) g_mode = XLineEditHEX::MODE_32;
    else if (_mode == XBinary::MODE_64) g_mode = XLineEditHEX::MODE_64;

    QAbstractItemModel *pOldModel = ui->tableViewMemoryMap->model();

    qint32 nNumberOfRecords = 0;

    bool bShowAll = ui->checkBoxShowAll->isChecked();

    if (bShowAll) {
        nNumberOfRecords = g_memoryMap.listRecords.count();
    } else {
        nNumberOfRecords = XBinary::getNumberOfPhysicalRecords(&g_memoryMap);
    }

    QStandardItemModel *pModel = new QStandardItemModel(nNumberOfRecords, 4);

    pModel->setHeaderData(0, Qt::Horizontal, tr("Offset"));
    pModel->setHeaderData(1, Qt::Horizontal, tr("Address"));
    pModel->setHeaderData(2, Qt::Horizontal, tr("Size"));
    pModel->setHeaderData(3, Qt::Horizontal, tr("Name"));

    //    QColor colDisabled = QWidget::palette().color(QPalette::Window);

    qint32 _nNumberOfRecords = g_memoryMap.listRecords.count();

    for (qint32 i = 0, j = 0; i < _nNumberOfRecords; i++) {
        //        bool bIsVirtual=g_memoryMap.listRecords.at(i).bIsVirtual;

        if ((!(g_memoryMap.listRecords.at(i).bIsVirtual)) || (bShowAll)) {
            g_mapIndexes.insert(i, j);

            QStandardItem *pItemOffset = new QStandardItem;

            pItemOffset->setData(g_memoryMap.listRecords.at(i).nOffset, Qt::UserRole + 0);
            pItemOffset->setData(g_memoryMap.listRecords.at(i).nAddress, Qt::UserRole + 1);
            pItemOffset->setData(g_memoryMap.listRecords.at(i).nSize, Qt::UserRole + 2);
            pItemOffset->setData(QString("%1_%2_%3.bin")
                                     .arg(XBinary::valueToHexEx(g_memoryMap.listRecords.at(i).nOffset), XBinary::valueToHexEx(g_memoryMap.listRecords.at(i).nSize),
                                          g_memoryMap.listRecords.at(i).sName),
                                 Qt::UserRole + 3);

            if (g_memoryMap.listRecords.at(i).nOffset != -1) {
                pItemOffset->setText(XLineEditHEX::getFormatString(g_mode, g_memoryMap.listRecords.at(i).nOffset));
            } else {
                //                pItemOffset->setBackground(colDisabled);
            }

            pModel->setItem(j, 0, pItemOffset);

            QStandardItem *pItemAddress = new QStandardItem;

            if (g_memoryMap.listRecords.at(i).nAddress != (quint64)-1) {
                pItemAddress->setText(XLineEditHEX::getFormatString(g_mode, g_memoryMap.listRecords.at(i).nAddress));
            } else {
                //                pItemAddress->setBackground(colDisabled);
            }

            pModel->setItem(j, 1, pItemAddress);

            QStandardItem *pItemSize = new QStandardItem;

            pItemSize->setText(XLineEditHEX::getFormatString(g_mode, g_memoryMap.listRecords.at(i).nSize));

            pModel->setItem(j, 2, pItemSize);

            QStandardItem *pItemName = new QStandardItem;

            pItemName->setText(g_memoryMap.listRecords.at(i).sName);
            pModel->setItem(j, 3, pItemName);

            j++;
        }
    }

    XOptions::setModelTextAlignment(pModel, 0, Qt::AlignRight | Qt::AlignVCenter);
    XOptions::setModelTextAlignment(pModel, 1, Qt::AlignRight | Qt::AlignVCenter);
    XOptions::setModelTextAlignment(pModel, 2, Qt::AlignRight | Qt::AlignVCenter);
    XOptions::setModelTextAlignment(pModel, 3, Qt::AlignLeft | Qt::AlignVCenter);

    ui->tableViewMemoryMap->setModel(pModel);

    deleteOldAbstractModel(&pOldModel);

    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    qint32 nColumnSize = XLineEditHEX::getWidthFromMode(this, g_mode);

    ui->tableViewMemoryMap->setColumnWidth(0, nColumnSize);
    ui->tableViewMemoryMap->setColumnWidth(1, nColumnSize);
    ui->tableViewMemoryMap->setColumnWidth(2, nColumnSize);

    connect(ui->tableViewMemoryMap->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
            SLOT(on_tableViewSelection(QItemSelection, QItemSelection)));
    connect(ui->widgetHex, SIGNAL(cursorViewOffsetChanged(qint64)), this, SLOT(onHexCursorChanged(qint64)));

    _adjust(true);

    ui->lineEditFileOffset->blockSignals(bBlocked1);
    ui->lineEditVirtualAddress->blockSignals(bBlocked2);
    ui->lineEditRelativeVirtualAddress->blockSignals(bBlocked3);
    ui->tableViewMemoryMap->blockSignals(bBlocked4);
    ui->pageHex->blockSignals(bBlocked5);
}

void XMemoryMapWidget::_adjust(bool bInit)
{
    const bool bBlocked1 = ui->lineEditFileOffset->blockSignals(true);
    const bool bBlocked2 = ui->lineEditVirtualAddress->blockSignals(true);
    const bool bBlocked3 = ui->lineEditRelativeVirtualAddress->blockSignals(true);
    const bool bBlocked4 = ui->tableViewMemoryMap->blockSignals(true);
    const bool bBlocked5 = ui->pageHex->blockSignals(true);

    qint32 nTableViewIndex = -1;

    quint64 nFileOffset = ui->lineEditFileOffset->getValue();
    XADDR nVirtualAddress = ui->lineEditVirtualAddress->getValue();
    quint64 nRelativeVirtualAddress = ui->lineEditRelativeVirtualAddress->getValue();

    if (ui->radioButtonFileOffset->isChecked()) {
        ui->lineEditFileOffset->setReadOnly(false);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nVirtualAddress = XBinary::offsetToAddress(&g_memoryMap, nFileOffset);
        nRelativeVirtualAddress = XBinary::offsetToRelAddress(&g_memoryMap, nFileOffset);

        XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByOffset(&g_memoryMap, nFileOffset);

        if (memoryRecord.nSize) {
            nTableViewIndex = memoryRecord.nIndex;
        }

        if (bInit) {
            ui->lineEditFileOffset->setModeValue(g_mode, nFileOffset);
        }

        ui->lineEditVirtualAddress->setModeValue(g_mode, nVirtualAddress);
        ui->lineEditRelativeVirtualAddress->setModeValue(g_mode, nRelativeVirtualAddress);
    } else if (ui->radioButtonVirtualAddress->isChecked()) {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(false);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nFileOffset = XBinary::addressToOffset(&g_memoryMap, nVirtualAddress);
        nRelativeVirtualAddress = XBinary::addressToRelAddress(&g_memoryMap, nVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByAddress(&g_memoryMap, nVirtualAddress);

        if (memoryRecord.nSize) {
            nTableViewIndex = memoryRecord.nIndex;
        }

        if (bInit) {
            ui->lineEditVirtualAddress->setModeValue(g_mode, nVirtualAddress);
        }

        ui->lineEditFileOffset->setModeValue(g_mode, nFileOffset);
        ui->lineEditRelativeVirtualAddress->setModeValue(g_mode, nRelativeVirtualAddress);
    } else if (ui->radioButtonRelativeVirtualAddress->isChecked()) {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(false);

        nFileOffset = XBinary::relAddressToOffset(&g_memoryMap, nRelativeVirtualAddress);
        nVirtualAddress = XBinary::relAddressToAddress(&g_memoryMap, nRelativeVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByRelAddress(&g_memoryMap, nRelativeVirtualAddress);

        if (memoryRecord.nSize) {
            nTableViewIndex = memoryRecord.nIndex;
        }

        if (bInit) {
            ui->lineEditRelativeVirtualAddress->setModeValue(g_mode, nRelativeVirtualAddress);
        }

        ui->lineEditFileOffset->setModeValue(g_mode, nFileOffset);
        ui->lineEditVirtualAddress->setModeValue(g_mode, nVirtualAddress);
    }

    if (nTableViewIndex != -1) {
        qint32 nIndex = g_mapIndexes.value(nTableViewIndex, -1);

        if (nIndex == -1) {
            QMessageBox::information(this, tr("Information"), tr("Virtual address"));
            nIndex = 0;
        }

        QModelIndex miCurrentIndex = ui->tableViewMemoryMap->model()->index(nIndex, 0);
        ui->tableViewMemoryMap->setCurrentIndex(miCurrentIndex);
    }

    _goToOffset(nFileOffset, 1);

    ui->lineEditFileOffset->blockSignals(bBlocked1);
    ui->lineEditVirtualAddress->blockSignals(bBlocked2);
    ui->lineEditRelativeVirtualAddress->blockSignals(bBlocked3);
    ui->tableViewMemoryMap->blockSignals(bBlocked4);
    ui->pageHex->blockSignals(bBlocked5);
}

void XMemoryMapWidget::on_lineEditFileOffset_textChanged(const QString &sText)
{
    Q_UNUSED(sText)

    _adjust(false);
}

void XMemoryMapWidget::on_lineEditVirtualAddress_textChanged(const QString &sText)
{
    Q_UNUSED(sText)

    _adjust(false);
}

void XMemoryMapWidget::on_lineEditRelativeVirtualAddress_textChanged(const QString &sText)
{
    Q_UNUSED(sText)

    _adjust(false);
}

void XMemoryMapWidget::on_tableViewSelection(const QItemSelection &itemSelected, const QItemSelection &itemDeselected)
{
    Q_UNUSED(itemSelected)
    Q_UNUSED(itemDeselected)

    viewSelection();
}

void XMemoryMapWidget::_goToOffset(qint64 nOffset, qint64 nSize)
{
    if (!g_bLockHex) {
        if (nSize == 0) {
            nSize = 1;
        }

        if (XBinary::isOffsetValid(&g_memoryMap, nOffset)) {
            ui->stackedWidgetHex->setCurrentIndex(0);

            ui->widgetHex->goToOffset(nOffset);
            ui->widgetHex->setDeviceSelection(nOffset, nSize);
            ui->widgetHex->reload();
        } else {
            // Invalid offset
            ui->stackedWidgetHex->setCurrentIndex(1);  // TODO Consts
        }
    }
}

void XMemoryMapWidget::onHexCursorChanged(qint64 nOffset)
{
    g_bLockHex = true;  // TODO mb use SignalBlocker

    if (!ui->lineEditFileOffset->isFocused()) {
        ui->lineEditFileOffset->setModeValue(g_mode, nOffset);
    }

    g_bLockHex = false;
}

void XMemoryMapWidget::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
    // mb TODO
}

void XMemoryMapWidget::on_pushButtonSave_clicked()
{
    XShortcutsWidget::saveTableModel(ui->tableViewMemoryMap->model(), XBinary::getResultFileName(g_pDevice, QString("%1.txt").arg(tr("Memory map"))));
}

void XMemoryMapWidget::on_checkBoxShowAll_stateChanged(int nValue)
{
    Q_UNUSED(nValue)

    updateMemoryMap();
}

void XMemoryMapWidget::on_pushButtonDumpAll_clicked()
{
    QString sDirectory = QFileDialog::getExistingDirectory(this, tr("Dump all"), XBinary::getDeviceDirectory(g_pDevice));

    if (!sDirectory.isEmpty()) {
        qint32 nNumberOfRecords = ui->tableViewMemoryMap->model()->rowCount();

        if (nNumberOfRecords) {
            QList<DumpProcess::RECORD> listRecords;

            for (qint32 i = 0; i < nNumberOfRecords; i++) {
                QModelIndex index = ui->tableViewMemoryMap->model()->index(i, 0);

                DumpProcess::RECORD record = {};

                record.nOffset = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 0).toLongLong();
                record.nSize = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 2).toLongLong();
                record.sFileName = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 3).toString();

                record.sFileName = sDirectory + QDir::separator() + QFileInfo(record.sFileName).fileName();

                listRecords.append(record);
            }

            DialogDumpProcess dd(this);

            dd.setData(g_pDevice, listRecords, DumpProcess::DT_OFFSET);

            dd.showDialogDelay();
        }
    }
}

void XMemoryMapWidget::on_tableViewMemoryMap_customContextMenuRequested(const QPoint &pos)
{
    qint32 nRow = ui->tableViewMemoryMap->currentIndex().row();

    if (nRow != -1) {
        QMenu contextMenu(this);

        QAction actionDump(tr("Dump to file"), this);
        connect(&actionDump, SIGNAL(triggered()), this, SLOT(dumpSection()));
        contextMenu.addAction(&actionDump);

        contextMenu.addMenu(getShortcuts()->getRowCopyMenu(this, ui->tableViewMemoryMap));

        contextMenu.exec(ui->tableViewMemoryMap->viewport()->mapToGlobal(pos));
    }
}

void XMemoryMapWidget::dumpSection()
{
    qint32 nRow = ui->tableViewMemoryMap->currentIndex().row();

    if (nRow != -1) {
        QModelIndex index = ui->tableViewMemoryMap->selectionModel()->selectedIndexes().at(0);

        qint64 nOffset = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 0).toLongLong();
        qint64 nSize = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 2).toLongLong();
        QString sName = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 3).toString();

        if (sName == "") {
            sName = tr("Dump");
        }

        QString sSaveFileName = XBinary::getResultFileName(g_pDevice, QString("%1.bin").arg(sName));
        QString sFileName = QFileDialog::getSaveFileName(this, tr("Save dump"), sSaveFileName, QString("%1 (*.bin)").arg(tr("Raw data")));

        if (!sFileName.isEmpty()) {
            DialogDumpProcess dd(this, g_pDevice, nOffset, nSize, sFileName, DumpProcess::DT_OFFSET);

            dd.showDialogDelay();
        }
    }
}

void XMemoryMapWidget::on_tableViewMemoryMap_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    viewSelection();
}

void XMemoryMapWidget::viewSelection()
{
    const bool bBlocked1 = ui->lineEditFileOffset->blockSignals(true);
    const bool bBlocked2 = ui->lineEditVirtualAddress->blockSignals(true);
    const bool bBlocked3 = ui->lineEditRelativeVirtualAddress->blockSignals(true);
    const bool bBlocked4 = ui->tableViewMemoryMap->blockSignals(true);
    const bool bBlocked5 = ui->pageHex->blockSignals(true);

    QItemSelectionModel *pSelectionModel = ui->tableViewMemoryMap->selectionModel();

    if (pSelectionModel) {
        QModelIndexList listIndexes = pSelectionModel->selectedRows(0);

        if (listIndexes.count()) {
            qint64 nFileOffset = listIndexes.at(0).data(Qt::UserRole + 0).toLongLong();
            XADDR nVirtualAddress = listIndexes.at(0).data(Qt::UserRole + 1).toLongLong();
            qint64 nSize = listIndexes.at(0).data(Qt::UserRole + 2).toLongLong();

            qint64 nRelativeVirtualAddress = XBinary::addressToRelAddress(&g_memoryMap, nVirtualAddress);

            ui->lineEditFileOffset->setModeValue(g_mode, nFileOffset);
            ui->lineEditVirtualAddress->setModeValue(g_mode, nVirtualAddress);
            ui->lineEditRelativeVirtualAddress->setModeValue(g_mode, nRelativeVirtualAddress);

            _goToOffset(nFileOffset, nSize);
        }
    }

    ui->lineEditFileOffset->blockSignals(bBlocked1);
    ui->lineEditVirtualAddress->blockSignals(bBlocked2);
    ui->lineEditRelativeVirtualAddress->blockSignals(bBlocked3);
    ui->tableViewMemoryMap->blockSignals(bBlocked4);
    ui->pageHex->blockSignals(bBlocked5);
}

void XMemoryMapWidget::on_pushButtonFileOffsetFind_clicked()
{
    quint64 nValue = ui->lineEditFileOffset->getValue();

    emit findValue(nValue, g_memoryMap.bIsBigEndian);
}

void XMemoryMapWidget::on_pushButtonVirtualAddressFind_clicked()
{
    quint64 nValue = ui->lineEditVirtualAddress->getValue();

    emit findValue(nValue, g_memoryMap.bIsBigEndian);
}

void XMemoryMapWidget::on_pushButtonRelativeVirtualAddressFind_clicked()
{
    quint64 nValue = ui->lineEditRelativeVirtualAddress->getValue();

    emit findValue(nValue, g_memoryMap.bIsBigEndian);
}
