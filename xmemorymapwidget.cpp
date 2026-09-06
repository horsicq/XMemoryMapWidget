/* Copyright (c) 2020-2026 hors<horsicq@gmail.com>
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

    XOptions::adjustToolButton(ui->toolButtonSave, XOptions::ICONTYPE_SAVE);
    XOptions::adjustToolButton(ui->toolButtonDumpAll, XOptions::ICONTYPE_DUMPTOFILE);
    XOptions::adjustToolButton(ui->toolButtonReload, XOptions::ICONTYPE_RELOAD);
    XOptions::adjustToolButton(ui->toolButtonFileOffsetFind, XOptions::ICONTYPE_SEARCH, Qt::ToolButtonIconOnly);
    XOptions::adjustToolButton(ui->toolButtonVirtualAddressFind, XOptions::ICONTYPE_SEARCH, Qt::ToolButtonIconOnly);
    XOptions::adjustToolButton(ui->toolButtonRelativeVirtualAddressFind, XOptions::ICONTYPE_SEARCH, Qt::ToolButtonIconOnly);

    ui->toolButtonSave->setToolTip(tr("Save"));
    ui->toolButtonDumpAll->setToolTip(tr("Dump all"));
    ui->toolButtonReload->setToolTip(tr("Reload"));
    ui->toolButtonFileOffsetFind->setToolTip(tr("Find"));
    ui->toolButtonVirtualAddressFind->setToolTip(tr("Find"));
    ui->toolButtonRelativeVirtualAddressFind->setToolTip(tr("Find"));
    ui->comboBoxType->setToolTip(tr("Type"));
    ui->comboBoxMapMode->setToolTip(tr("Mode"));
    ui->lineEditArch->setToolTip(tr("Architecture"));
    ui->lineEditMode->setToolTip(tr("Mode"));
    ui->lineEditEndianness->setToolTip(tr("Endianness"));
    ui->checkBoxShowAll->setToolTip(tr("Show all"));
    ui->tableViewMemoryMap->setToolTip(tr("Memory map"));
    ui->lineEditFileOffset->setToolTip(tr("File offset"));
    ui->lineEditVirtualAddress->setToolTip(tr("Virtual address"));
    ui->lineEditRelativeVirtualAddress->setToolTip(tr("Relative virtual address"));

    m_inData = {};
    m_options = {};
    m_mode = XLineEditValidator::MODE_HEX_16;
    m_bLockHex = false;
    m_bLockSelection = false;
    m_memoryMap = {};
    m_pXInfoDB = nullptr;

    ui->checkBoxShowAll->setChecked(true);

    _adjustLineEditWidths();
}

XMemoryMapWidget::~XMemoryMapWidget()
{
    ui->widgetHex->reset();
    XFormats::removeDevice(m_inData.pDevice, m_inData);
    delete ui;
}

void XMemoryMapWidget::setData(const XBinary::INDATA &inData, const OPTIONS &options, XInfoDB *pXInfoDB)
{
    ui->widgetHex->reset();
    XFormats::removeDevice(m_inData.pDevice, m_inData);
    m_inData = inData;
    m_inData.pDevice = XFormats::createDevice(inData);
    m_options = options;
    m_pXInfoDB = pXInfoDB;

    XBinaryView::OPTIONS hex_options = {};  // TODO Check !!!

    ui->widgetHex->setData(m_inData.pDevice, hex_options, true, pXInfoDB);

    if (m_inData.pDevice) {
        XFormats::setFileTypeComboBox(options.fileType, m_inData.pDevice, ui->comboBoxType);
        XFormats::getMapModesList(options.fileType, ui->comboBoxMapMode);

        updateMemoryMap();
    }

    if (options.bIsSearchEnable) {
        ui->toolButtonFileOffsetFind->show();
        ui->toolButtonRelativeVirtualAddressFind->show();
        ui->toolButtonVirtualAddressFind->show();
    } else {
        ui->toolButtonFileOffsetFind->hide();
        ui->toolButtonRelativeVirtualAddressFind->hide();
        ui->toolButtonVirtualAddressFind->hide();
    }
}

void XMemoryMapWidget::setData(QIODevice *pDevice, const OPTIONS &options, XInfoDB *pXInfoDB)
{
    setData(XFormats::createINDATA(options.fileType, pDevice), options, pXInfoDB);
}

QIODevice *XMemoryMapWidget::getDevice()
{
    return m_inData.pDevice;
}

void XMemoryMapWidget::setXInfoDB(XInfoDB *pXInfoDB)
{
    m_pXInfoDB = pXInfoDB;
    ui->widgetHex->setXInfoDB(pXInfoDB);
}

void XMemoryMapWidget::goToOffset(qint64 nOffset)
{
    ui->lineEditFileOffset->setValidatorModeValue(m_mode, nOffset);
}

void XMemoryMapWidget::setLocation(quint64 nLocation, qint32 nLocationType, qint64 nSize)
{
    Q_UNUSED(nSize)

    // The matching radio button must be active first, otherwise _adjust()
    // recomputes the fields from the previously active one and the requested
    // location is discarded.
    if (nLocationType == XBinary::LT_ADDRESS) {
        ui->radioButtonVirtualAddress->setChecked(true);
        ui->lineEditVirtualAddress->setValidatorModeValue(m_mode, nLocation);
    } else if (nLocationType == XBinary::LT_OFFSET) {
        ui->radioButtonFileOffset->setChecked(true);
        ui->lineEditFileOffset->setValidatorModeValue(m_mode, nLocation);
    }
}

void XMemoryMapWidget::setGlobal(XShortcuts *pShortcuts, XOptions *pXOptions)
{
    ui->widgetHex->setGlobal(pShortcuts, pXOptions);
    XShortcutsWidget::setGlobal(pShortcuts, pXOptions);
}

void XMemoryMapWidget::adjustView()
{
    ui->widgetHex->adjustView();
    getGlobalOptions()->adjustWidget(this, XOptions::ID_VIEW_FONT_CONTROLS);
    getGlobalOptions()->adjustTableView(ui->tableViewMemoryMap, XOptions::ID_VIEW_FONT_TABLEVIEWS);
    _adjustLineEditWidths();
}

void XMemoryMapWidget::reloadData(bool bSaveSelection)
{
    Q_UNUSED(bSaveSelection)
    updateMemoryMap();
}

void XMemoryMapWidget::on_comboBoxType_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    XBinary::FT fileType = (XBinary::FT)(ui->comboBoxType->currentData().toInt());
    XFormats::getMapModesList(fileType, ui->comboBoxMapMode);

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
    if (m_inData.pDevice) {
        const bool bBlocked1 = ui->lineEditFileOffset->blockSignals(true);
        const bool bBlocked2 = ui->lineEditVirtualAddress->blockSignals(true);
        const bool bBlocked3 = ui->lineEditRelativeVirtualAddress->blockSignals(true);
        const bool bBlocked4 = ui->tableViewMemoryMap->blockSignals(true);
        const bool bBlocked5 = ui->pageHex->blockSignals(true);

        m_mapIndexes.clear();

        quint64 nSavedOffset = ui->lineEditFileOffset->getValue_uint64();

        XBinary::FT fileType = (XBinary::FT)(ui->comboBoxType->currentData().toInt());
        XBinary::MAPMODE mapMode = (XBinary::MAPMODE)(ui->comboBoxMapMode->currentData().toInt());

        m_memoryMap = XFormats::getMemoryMap(fileType, mapMode, m_inData.pDevice);

        ui->lineEditArch->setText(m_memoryMap.sArch);
        ui->lineEditMode->setText(XBinary::modeIdToString(m_memoryMap.mode));
        ui->lineEditEndianness->setText(XBinary::endianToString(m_memoryMap.endian));

        ui->radioButtonFileOffset->setChecked(true);

        XBinary::MODE _mode = XBinary::getWidthModeFromMemoryMap(&m_memoryMap);

        // TODO move function to XShortcutWidget !!!
        if (_mode == XBinary::MODE_8) m_mode = XLineEditValidator::MODE_HEX_8;
        else if (_mode == XBinary::MODE_16) m_mode = XLineEditValidator::MODE_HEX_16;
        else if (_mode == XBinary::MODE_32) m_mode = XLineEditValidator::MODE_HEX_32;
        else if (_mode == XBinary::MODE_64) m_mode = XLineEditValidator::MODE_HEX_64;

        _adjustLineEditWidths();

        // Keep the position across reloads, show-all toggles and map mode
        // switches; fall back to the start when it no longer maps.
        if (!XBinary::isOffsetValid(&m_memoryMap, (qint64)nSavedOffset)) {
            nSavedOffset = 0;
        }

        ui->lineEditFileOffset->setValidatorModeValue(m_mode, nSavedOffset);

        qint32 nNumberOfRecords = 0;

        bool bShowAll = ui->checkBoxShowAll->isChecked();

        if (bShowAll) {
            nNumberOfRecords = m_memoryMap.listRecords.count();
        } else {
            nNumberOfRecords = XBinary::getNumberOfPhysicalRecords(&m_memoryMap);
        }

        QStandardItemModel *pModel = new QStandardItemModel(nNumberOfRecords, 4);

        pModel->setHeaderData(0, Qt::Horizontal, tr("Offset"));
        pModel->setHeaderData(1, Qt::Horizontal, tr("Address"));
        pModel->setHeaderData(2, Qt::Horizontal, tr("Size"));
        pModel->setHeaderData(3, Qt::Horizontal, tr("Name"));

        // Virtual records (no backing file data) are listed dimmed.
        QBrush brushVirtual(QWidget::palette().color(QPalette::Disabled, QPalette::WindowText));

        qint32 _nNumberOfRecords = m_memoryMap.listRecords.count();

        for (qint32 i = 0, j = 0; i < _nNumberOfRecords; i++) {
            bool bIsVirtual = m_memoryMap.listRecords.at(i).bIsVirtual;

            if ((!bIsVirtual) || (bShowAll)) {
                m_mapIndexes.insert(i, j);

                QStandardItem *pItemOffset = new QStandardItem;

                pItemOffset->setData(m_memoryMap.listRecords.at(i).nOffset, Qt::UserRole + 0);
                pItemOffset->setData(m_memoryMap.listRecords.at(i).nAddress, Qt::UserRole + 1);
                pItemOffset->setData(m_memoryMap.listRecords.at(i).nSize, Qt::UserRole + 2);
                pItemOffset->setData(QString("%1_%2_%3.bin")
                                         .arg(XBinary::valueToHexEx(m_memoryMap.listRecords.at(i).nOffset), XBinary::valueToHexEx(m_memoryMap.listRecords.at(i).nSize),
                                              m_memoryMap.listRecords.at(i).sName),
                                     Qt::UserRole + 3);

                if (m_memoryMap.listRecords.at(i).nOffset != -1) {
                    pItemOffset->setText(XLineEditHEX::getFormatString(m_mode, m_memoryMap.listRecords.at(i).nOffset));
                }

                pModel->setItem(j, 0, pItemOffset);

                QStandardItem *pItemAddress = new QStandardItem;

                if (m_memoryMap.listRecords.at(i).nAddress != (quint64)-1) {
                    pItemAddress->setText(XLineEditHEX::getFormatString(m_mode, m_memoryMap.listRecords.at(i).nAddress));
                }

                pModel->setItem(j, 1, pItemAddress);

                QStandardItem *pItemSize = new QStandardItem;

                pItemSize->setText(XLineEditHEX::getFormatString(m_mode, m_memoryMap.listRecords.at(i).nSize));

                pModel->setItem(j, 2, pItemSize);

                QStandardItem *pItemName = new QStandardItem;

                pItemName->setText(m_memoryMap.listRecords.at(i).sName);
                pModel->setItem(j, 3, pItemName);

                if (bIsVirtual) {
                    pItemOffset->setForeground(brushVirtual);
                    pItemAddress->setForeground(brushVirtual);
                    pItemSize->setForeground(brushVirtual);
                    pItemName->setForeground(brushVirtual);
                }

                j++;
            }
        }

        XOptions::setModelTextAlignment(pModel, 0, Qt::AlignRight | Qt::AlignVCenter);
        XOptions::setModelTextAlignment(pModel, 1, Qt::AlignRight | Qt::AlignVCenter);
        XOptions::setModelTextAlignment(pModel, 2, Qt::AlignRight | Qt::AlignVCenter);
        XOptions::setModelTextAlignment(pModel, 3, Qt::AlignLeft | Qt::AlignVCenter);

        ui->tableViewMemoryMap->setCustomModel(pModel, true);

        ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
        ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
        ui->tableViewMemoryMap->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

        qint32 nColumnSize = XLineEditHEX::getWidthFromMode(this, m_mode);

        // The mode width fits the values; a small file (narrow mode) must still
        // show the whole column title, so never go below the header's own hint.
        for (qint32 i = 0; i < 3; i++) {
            qint32 nHeaderWidth = ui->tableViewMemoryMap->horizontalHeader()->sectionSizeHint(i);
            ui->tableViewMemoryMap->setColumnWidth(i, qMax(nColumnSize, nHeaderWidth));
        }

        connect(ui->tableViewMemoryMap->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
                SLOT(on_tableViewSelection(QItemSelection, QItemSelection)));
        // widgetHex is persistent across refreshes; UniqueConnection stops the slot stacking up on every updateMemoryMap().
        connect(ui->widgetHex, SIGNAL(cursorViewPosChanged(qint64)), this, SLOT(onHexCursorChanged(qint64)), Qt::UniqueConnection);

        _adjust(true);

        ui->lineEditFileOffset->blockSignals(bBlocked1);
        ui->lineEditVirtualAddress->blockSignals(bBlocked2);
        ui->lineEditRelativeVirtualAddress->blockSignals(bBlocked3);
        ui->tableViewMemoryMap->blockSignals(bBlocked4);
        ui->pageHex->blockSignals(bBlocked5);
    }
}

void XMemoryMapWidget::_adjust(bool bInit)
{
    const bool bBlocked1 = ui->lineEditFileOffset->blockSignals(true);
    const bool bBlocked2 = ui->lineEditVirtualAddress->blockSignals(true);
    const bool bBlocked3 = ui->lineEditRelativeVirtualAddress->blockSignals(true);
    const bool bBlocked4 = ui->tableViewMemoryMap->blockSignals(true);
    const bool bBlocked5 = ui->pageHex->blockSignals(true);

    qint32 nTableViewIndex = -1;

    quint64 nFileOffset = ui->lineEditFileOffset->getValue_uint64();
    XADDR nVirtualAddress = ui->lineEditVirtualAddress->getValue_uint64();
    quint64 nRelativeVirtualAddress = ui->lineEditRelativeVirtualAddress->getValue_uint64();

    if (ui->radioButtonFileOffset->isChecked()) {
        ui->lineEditFileOffset->setReadOnly(false);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nVirtualAddress = XBinary::offsetToAddress(&m_memoryMap, nFileOffset);
        nRelativeVirtualAddress = XBinary::offsetToRelAddress(&m_memoryMap, nFileOffset);

        XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByOffset(&m_memoryMap, nFileOffset);

        if (memoryRecord.nSize) {
            nTableViewIndex = memoryRecord.nIndex;
        }

        if (bInit) {
            ui->lineEditFileOffset->setValidatorModeValue(m_mode, nFileOffset);
        }

        _setLocationValue(ui->lineEditVirtualAddress, nVirtualAddress);
        _setLocationValue(ui->lineEditRelativeVirtualAddress, nRelativeVirtualAddress);
    } else if (ui->radioButtonVirtualAddress->isChecked()) {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(false);
        ui->lineEditRelativeVirtualAddress->setReadOnly(true);

        nFileOffset = XBinary::addressToOffset(&m_memoryMap, nVirtualAddress);
        nRelativeVirtualAddress = XBinary::addressToRelAddress(&m_memoryMap, nVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByAddress(&m_memoryMap, nVirtualAddress);

        if (memoryRecord.nSize) {
            nTableViewIndex = memoryRecord.nIndex;
        }

        if (bInit) {
            ui->lineEditVirtualAddress->setValidatorModeValue(m_mode, nVirtualAddress);
        }

        _setLocationValue(ui->lineEditFileOffset, nFileOffset);
        _setLocationValue(ui->lineEditRelativeVirtualAddress, nRelativeVirtualAddress);
    } else if (ui->radioButtonRelativeVirtualAddress->isChecked()) {
        ui->lineEditFileOffset->setReadOnly(true);
        ui->lineEditVirtualAddress->setReadOnly(true);
        ui->lineEditRelativeVirtualAddress->setReadOnly(false);

        nFileOffset = XBinary::relAddressToOffset(&m_memoryMap, nRelativeVirtualAddress);
        nVirtualAddress = XBinary::relAddressToAddress(&m_memoryMap, nRelativeVirtualAddress);

        XBinary::_MEMORY_RECORD memoryRecord = XBinary::getMemoryRecordByRelAddress(&m_memoryMap, nRelativeVirtualAddress);

        if (memoryRecord.nSize) {
            nTableViewIndex = memoryRecord.nIndex;
        }

        if (bInit) {
            ui->lineEditRelativeVirtualAddress->setValidatorModeValue(m_mode, nRelativeVirtualAddress);
        }

        _setLocationValue(ui->lineEditFileOffset, nFileOffset);
        _setLocationValue(ui->lineEditVirtualAddress, nVirtualAddress);
    }

    // The selection model emits selectionChanged past the view's blockSignals;
    // without the lock, viewSelection() would snap the fields being typed back
    // to the start of the selected record.
    m_bLockSelection = true;

    if (nTableViewIndex != -1) {
        qint32 nIndex = m_mapIndexes.value(nTableViewIndex, -1);

        if (nIndex != -1) {
            QModelIndex miCurrentIndex = ui->tableViewMemoryMap->model()->index(nIndex, 0);
            ui->tableViewMemoryMap->setCurrentIndex(miCurrentIndex);
        } else {
            // The record is filtered out ("Show all" is off and the location is
            // virtual-only) - drop the selection instead of interrupting typing.
            ui->tableViewMemoryMap->clearSelection();
        }
    } else {
        ui->tableViewMemoryMap->clearSelection();
    }

    m_bLockSelection = false;

    _goToOffset(nFileOffset, 1);

    ui->lineEditFileOffset->blockSignals(bBlocked1);
    ui->lineEditVirtualAddress->blockSignals(bBlocked2);
    ui->lineEditRelativeVirtualAddress->blockSignals(bBlocked3);
    ui->tableViewMemoryMap->blockSignals(bBlocked4);
    ui->pageHex->blockSignals(bBlocked5);
}

void XMemoryMapWidget::_setLocationValue(XLineEditHEX *pLineEdit, quint64 nValue)
{
    // (quint64)-1 is the "not mapped" result of the offset<->address
    // conversions and of the virtual/unmapped memory records; show an
    // empty field for it instead of the sentinel formatted as ff.. in the
    // current width. The validator mode and the stored value (0) are kept
    // in step with the empty text.
    if (nValue != (quint64)-1) {
        pLineEdit->setValidatorModeValue(m_mode, nValue);
    } else {
        pLineEdit->setValidatorModeValue(m_mode, 0);
        pLineEdit->clear();
    }
}

void XMemoryMapWidget::_adjustLineEditWidths()
{
    // The edits are centre-aligned, so a text wider than the field loses
    // characters at both ends. Reserve room for the widest value of the
    // current mode (16 hex digits for 64-bit; XLineEditHEX shows non-zero
    // values in bold) plus the frame, style padding and text margins.
    QFont fontValue = ui->lineEditFileOffset->font();
    fontValue.setBold(true);
    QFontMetrics fmValue(fontValue);

    qint32 nNumberOfDigits = XLineEditValidator::getNumberOfBits(m_mode) / 4;
    qint32 nExtra = fmValue.horizontalAdvance(QString("WWW"));
    qint32 nValueWidth = fmValue.horizontalAdvance(QString(nNumberOfDigits, QChar('0'))) + nExtra;

    ui->lineEditFileOffset->setMinimumWidth(nValueWidth);
    ui->lineEditVirtualAddress->setMinimumWidth(nValueWidth);
    ui->lineEditRelativeVirtualAddress->setMinimumWidth(nValueWidth);

    // Mode / Endianness / Architecture hold short words; "Unknown" is the
    // widest one the first two can show, so size for it and the current
    // text (the edits have an Ignored horizontal policy, so this width is
    // what the group boxes are laid out from).
    QFontMetrics fmInfo(ui->lineEditMode->font());
    qint32 nUnknownModeWidth = fmInfo.horizontalAdvance(XBinary::modeIdToString(XBinary::MODE_UNKNOWN));
    qint32 nUnknownEndianWidth = fmInfo.horizontalAdvance(XBinary::endianToString(XBinary::ENDIAN_UNKNOWN));

    ui->lineEditMode->setMinimumWidth(qMax(fmInfo.horizontalAdvance(ui->lineEditMode->text()), nUnknownModeWidth) + nExtra);
    ui->lineEditEndianness->setMinimumWidth(qMax(fmInfo.horizontalAdvance(ui->lineEditEndianness->text()), nUnknownEndianWidth) + nExtra);
    ui->lineEditArch->setMinimumWidth(qMax(fmInfo.horizontalAdvance(ui->lineEditArch->text()), nUnknownModeWidth) + nExtra);
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

    if (!m_bLockSelection) {
        viewSelection();
    }
}

void XMemoryMapWidget::_goToOffset(qint64 nOffset, qint64 nSize)
{
    if (!m_bLockHex) {
        if (nSize == 0) {
            nSize = 1;
        }

        if (XBinary::isOffsetValid(&m_memoryMap, nOffset)) {
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
    m_bLockHex = true;  // TODO mb use SignalBlocker

    if (!ui->lineEditFileOffset->isFocused()) {
        ui->lineEditFileOffset->setValidatorModeValue(m_mode, nOffset);
    }

    m_bLockHex = false;
}

void XMemoryMapWidget::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
    // mb TODO
}

void XMemoryMapWidget::on_toolButtonSave_clicked()
{
    if (!m_inData.pDevice) {
        return;
    }

    XShortcutsWidget::saveTableModel(ui->tableViewMemoryMap->getProxyModel(), XBinary::getResultFileName(m_inData.pDevice, QString("%1.txt").arg(tr("Memory map"))));
}

void XMemoryMapWidget::on_checkBoxShowAll_stateChanged(int nValue)
{
    Q_UNUSED(nValue)

    updateMemoryMap();
}

void XMemoryMapWidget::on_toolButtonDumpAll_clicked()
{
    if (!m_inData.pDevice) {
        return;
    }

    QString sDirectory = QFileDialog::getExistingDirectory(this, tr("Dump all"), XBinary::getDeviceDirectory(m_inData.pDevice));

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

            QString sJsonFileName = sDirectory + QDir::separator() + XBinary::getDeviceFileBaseName(m_inData.pDevice) + ".patch.json";

            DumpProcess dumpProcess;
            XDialogProcess dd(this, &dumpProcess);
            dd.setGlobal(getShortcuts(), getGlobalOptions());
            dumpProcess.setData(m_inData.pDevice, listRecords, DumpProcess::DT_DUMP_DEVICE_OFFSET, sJsonFileName, dd.getPdStruct());
            dd.start();
            dd.showDialogDelay();
        }
    }
}

void XMemoryMapWidget::on_tableViewMemoryMap_customContextMenuRequested(const QPoint &pos)
{
    qint32 nRow = ui->tableViewMemoryMap->currentIndex().row();

    if (nRow != -1) {
        QMenu contextMenu(this);

        QList<XShortcuts::MENUITEM> listMenuItems;

        getShortcuts()->_addMenuItem(&listMenuItems, X_ID_TABLE_SELECTION_DUMPTOFILE, this, SLOT(dumpSection()), XShortcuts::GROUPID_NONE);
        getShortcuts()->_addMenuItem_CopyRow(&listMenuItems, ui->tableViewMemoryMap);

        getShortcuts()->adjustContextMenu(&contextMenu, &listMenuItems);

        contextMenu.exec(ui->tableViewMemoryMap->viewport()->mapToGlobal(pos));
    }
}

void XMemoryMapWidget::dumpSection()
{
    QModelIndexList listSelected = ui->tableViewMemoryMap->selectionModel()->selectedIndexes();

    if (!listSelected.isEmpty()) {
        QModelIndex index = listSelected.at(0);

        qint64 nOffset = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 0).toLongLong();
        qint64 nSize = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 2).toLongLong();
        QString sName = ui->tableViewMemoryMap->model()->data(index, Qt::UserRole + 3).toString();

        if (sName == "") {
            sName = tr("Dump");
        }

        QString sSaveFileName = XBinary::getResultFileName(m_inData.pDevice, QString("%1.bin").arg(sName));
        QString sFileName = QFileDialog::getSaveFileName(this, tr("Save dump"), sSaveFileName, QString("%1 (*.bin)").arg(tr("Raw data")));

        if (!sFileName.isEmpty()) {
            DumpProcess dumpProcess;
            XDialogProcess dd(this, &dumpProcess);
            dd.setGlobal(getShortcuts(), getGlobalOptions());
            dumpProcess.setData(m_inData.pDevice, nOffset, nSize, sFileName, DumpProcess::DT_DUMP_DEVICE_OFFSET, dd.getPdStruct());
            dd.start();
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

            qint64 nRelativeVirtualAddress = XBinary::addressToRelAddress(&m_memoryMap, nVirtualAddress);

            _setLocationValue(ui->lineEditFileOffset, nFileOffset);
            _setLocationValue(ui->lineEditVirtualAddress, nVirtualAddress);
            _setLocationValue(ui->lineEditRelativeVirtualAddress, nRelativeVirtualAddress);

            _goToOffset(nFileOffset, nSize);

            if (nFileOffset != -1) {
                emit currentLocationChanged(nFileOffset, XBinary::LT_OFFSET, nSize);
            } else if (nVirtualAddress != (XADDR)-1) {
                emit currentLocationChanged(nVirtualAddress, XBinary::LT_ADDRESS, nSize);
            }
        }
    }

    ui->lineEditFileOffset->blockSignals(bBlocked1);
    ui->lineEditVirtualAddress->blockSignals(bBlocked2);
    ui->lineEditRelativeVirtualAddress->blockSignals(bBlocked3);
    ui->tableViewMemoryMap->blockSignals(bBlocked4);
    ui->pageHex->blockSignals(bBlocked5);
}

void XMemoryMapWidget::on_toolButtonFileOffsetFind_clicked()
{
    quint64 nValue = ui->lineEditFileOffset->getValue_uint64();

    emit findValue(nValue, m_memoryMap.endian);
}

void XMemoryMapWidget::on_toolButtonVirtualAddressFind_clicked()
{
    quint64 nValue = ui->lineEditVirtualAddress->getValue_uint64();

    emit findValue(nValue, m_memoryMap.endian);
}

void XMemoryMapWidget::on_toolButtonRelativeVirtualAddressFind_clicked()
{
    quint64 nValue = ui->lineEditRelativeVirtualAddress->getValue_uint64();

    emit findValue(nValue, m_memoryMap.endian);
}

void XMemoryMapWidget::on_comboBoxMapMode_currentIndexChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    updateMemoryMap();
}

void XMemoryMapWidget::on_toolButtonReload_clicked()
{
    reloadData(true);
}
