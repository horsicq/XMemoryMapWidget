/* Copyright (c) 2017-2022 hors<horsicq@gmail.com>
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
#include "dialogmemorymap.h"
#include "ui_dialogmemorymap.h"

DialogMemoryMap::DialogMemoryMap(QWidget *pParent) :
    XShortcutsDialog(pParent),
    ui(new Ui::DialogMemoryMap)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
}

DialogMemoryMap::DialogMemoryMap(QWidget *pParent,QIODevice *pDevice,XBinary::FT fileType) :
    DialogMemoryMap(pParent)
{
    setData(pDevice,fileType);
}

DialogMemoryMap::~DialogMemoryMap()
{
    delete ui;
}

void DialogMemoryMap::setData(QIODevice *pDevice, XBinary::FT fileType)
{
    ui->widgetMemoryMap->setData(pDevice,fileType);
}

void DialogMemoryMap::setGlobal(XShortcuts *pShortcuts,XOptions *pXOptions)
{
    ui->widgetMemoryMap->setGlobal(pShortcuts,pXOptions);
    XShortcutsDialog::setGlobal(pShortcuts,pXOptions);
}

void DialogMemoryMap::on_pushButtonClose_clicked()
{
    this->close();
}
