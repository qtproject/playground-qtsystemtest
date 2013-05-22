/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of QtSystemTest.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtestbrowserdlg_p.h"
#include <QtSystemTest/QstConnection>

#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QVariantMap>
#include <QMetaMethod>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QCheckBox>
#include <QScrollArea>

QString signature(const QVariantMap &map)
{
    return map[QstConnection::signatureProperty].toString();
}

TestImageViewer::TestImageViewer(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
}

TestImageViewer::~TestImageViewer()
{
}

void TestImageViewer::setSelectedArea(const QRect& rect)
{
    m_selectedArea = rect;
    update();
}

void TestImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(event->pos());
    }
}

void TestImageViewer::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (m_selectedArea.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setPen(Qt::red);
    painter.drawRect(m_selectedArea);
}

QTestBrowserDlg::QTestBrowserDlg( QstConnection* test, const QVariantMap &item, const QVariantMap &imgItem )
    : QDialog(0, Qt::Window)
    , m_test(0)
    , m_image(0)
    , m_appContext(0)
    , m_tree(0)
    , m_queryTab(0)
    , m_propertyTab(0)
    , m_itemPropertyTable(0)
    , m_queryTable(0)
    , m_resultsTable(0)
    , m_contextTableItem(0)
    , m_flagInvisible(0)
    , m_flagDisabled(0)
    , m_flagIgnoreScan(0)
    , m_refreshAction(0)
    , m_expandAction(0)
    , m_collapseAction(0)
    , m_methodsAction(0)
    , m_propertyRefreshAction(0)
    , m_propertyMethodsAction(0)
    , m_propertyPropertiesAction(0)
    , m_propertyQueryAction(0)
    , m_propertyCopyAction(0)
    , m_propertyCopyTableAction(0)
    , m_queryClearAction(0)
    , m_queryCopyAction(0)
{
    setSystemTest(test);
    createActions();
    createWidgets();
    updateAppContext();
    setBaseItems(item, imgItem);
}

QTestBrowserDlg::~QTestBrowserDlg()
{
}

void QTestBrowserDlg::createActions()
{
    m_refreshAction = new QAction(tr("Refresh"), this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, SIGNAL(triggered()), this, SLOT(refresh()));
    addAction(m_refreshAction);

    m_expandAction = new QAction(tr("Expand Tree"), this);
    m_expandAction->setShortcut(QKeySequence(tr("Alt+E")));
    connect(m_expandAction, SIGNAL(triggered()), this, SLOT(expandAll()));
    addAction(m_expandAction);

    m_collapseAction = new QAction(tr("Collapse Tree"), this);
    m_collapseAction->setShortcut(QKeySequence(tr("Alt+C")));
    connect(m_collapseAction, SIGNAL(triggered()), this, SLOT(collapseAll()));
    addAction(m_collapseAction);

    m_methodsAction = new QAction(tr("View Methods"), this);
    m_methodsAction->setShortcut(QKeySequence(tr("Alt+M")));
    connect(m_methodsAction, SIGNAL(triggered()), this, SLOT(viewMethods()));
    addAction(m_methodsAction);

    m_propertyRefreshAction = new QAction(tr("Refresh Properties"), this);
    connect(m_propertyRefreshAction, SIGNAL(triggered()), this, SLOT(refreshProperties()));

    m_propertyMethodsAction = new QAction(this);
    connect(m_propertyMethodsAction, SIGNAL(triggered()), this, SLOT(viewPropertyMethods()));

    m_propertyPropertiesAction = new QAction(this);
    connect(m_propertyPropertiesAction, SIGNAL(triggered()), this, SLOT(viewPropertyProperties()));

    m_propertyQueryAction = new QAction(tr("Add as Query Condition"), this);
    connect(m_propertyQueryAction, SIGNAL(triggered()), this, SLOT(addPropertyAsQueryCondition()));

    m_propertyCopyAction = new QAction(tr("Copy"), this);
    connect(m_propertyCopyAction, SIGNAL(triggered()), this, SLOT(copyTableItemValue()));

    m_propertyCopyTableAction = new QAction(tr("Copy Table"), this);
    connect(m_propertyCopyTableAction, SIGNAL(triggered()), this, SLOT(copyTable()));

    m_queryClearAction = new QAction(tr("Clear Query"), this);
    connect(m_queryClearAction, SIGNAL(triggered()), this, SLOT(clearQuery()));

    m_queryCopyAction = new QAction(tr("Copy Query"), this);
    connect(m_queryCopyAction, SIGNAL(triggered()), this, SLOT(copyQuery()));

}

void QTestBrowserDlg::createWidgets()
{
    QHBoxLayout *mainLayout = new QHBoxLayout( this );
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);

    mainLayout->addWidget(mainSplitter);
    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    m_image = new TestImageViewer();
    m_image->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    connect(m_image, SIGNAL(clicked(QPoint)), this, SLOT(onImageClicked(QPoint)));

    m_image->setAlignment( Qt::AlignCenter );
    m_image->setFrameShape( QLabel::NoFrame );
    m_image->setScaledContents( false );

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(m_image);
    leftSplitter->addWidget(scrollArea);
    leftSplitter->setStretchFactor(0, 2);

    m_appContext = new QComboBox(this);
    m_appContext->installEventFilter(this);
    leftSplitter->addWidget(m_appContext);

    m_queryTab = createQueryTab();
    leftSplitter->addWidget(m_queryTab);

    m_tree = new QTreeWidget( this );
    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    m_tree->setColumnCount(6);
    m_tree->setHeaderLabels(QStringList() << QLatin1String("Type") << QLatin1String("Signature") << QLatin1String("QML id") << QLatin1String("objectName") << QLatin1String("Geometry") << QLatin1String("text"));

    rightSplitter->addWidget(m_tree);
    rightSplitter->setStretchFactor(0, 2);
    m_propertyTab = new QTabWidget(this);
    m_itemPropertyTable = new QTableWidget(this);
    initPropertyTable(m_itemPropertyTable);

    m_propertyTab->addTab(m_itemPropertyTable, QLatin1String("Item Properties"));
    m_propertyTab->setTabEnabled(0, false);
    m_propertyTab->setTabPosition(QTabWidget::South);
    m_propertyTab->setTabsClosable(true);
    connect(m_propertyTab, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));

    rightSplitter->addWidget(m_propertyTab);

    setWindowTitle( "Object Browser" );
    m_tabItemSignatures.append("");
}

QTabWidget * QTestBrowserDlg::createQueryTab()
{
    QTabWidget *tabWidget = new QTabWidget(this);
    QFrame *queryFrame = new QFrame(this);
    QFrame *flagsFrame = new QFrame(this);
    QVBoxLayout *queryLayout = new QVBoxLayout(queryFrame);
    QVBoxLayout *flagsLayout = new QVBoxLayout(flagsFrame);

    m_queryTable = new QTableWidget(this);
    static QStringList labels(QStringList() << QString() << QLatin1String("Property") << QLatin1String("Value"));
    m_queryTable->setColumnCount(labels.count());
    m_queryTable->verticalHeader()->hide();
    m_queryTable->horizontalHeader()->setStretchLastSection(true);
    m_queryTable->setHorizontalHeaderLabels(labels);
    m_queryTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_queryTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_queryTable->setRowCount(1);
    addQueryRow();
    m_queryTable->resizeColumnsToContents();
    m_queryTable->setSpan(1, 0, 1, labels.count());
    QPushButton *addButton = new QPushButton(tr("Add New Condition"));
    m_queryTable->setCellWidget(1, 0, addButton);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addQueryRow()) );
    connect(m_queryTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onQueryItemChanged(QTableWidgetItem*)));
    connect(m_queryTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onQueryContextMenu(QPoint)));
    QPushButton *queryButton = new QPushButton("Query");
    connect( queryButton, SIGNAL(clicked()), this, SLOT(doQuery()) );
    queryLayout->addWidget(m_queryTable);
    queryLayout->addWidget(queryButton);

    m_flagInvisible = new QCheckBox(QLatin1String("Scan Invisible Items"), this);
    m_flagDisabled = new QCheckBox(QLatin1String("Scan Disabled Items"), this);
    m_flagIgnoreScan = new QCheckBox(QLatin1String("Ignore Scanned"), this);
    QLabel *flagsInfo = new QLabel(QLatin1String("Query Flags are used to control how the search is performed. Please refer to the QtUiTest documentation for details."));
    flagsInfo->setWordWrap(true);
    flagsLayout->addWidget(m_flagInvisible);
    flagsLayout->addWidget(m_flagDisabled);
    flagsLayout->addWidget(m_flagIgnoreScan);
    flagsLayout->addWidget(flagsInfo);

    m_resultsTable = new QTableWidget(this);
    static QStringList resultLabels(QStringList() << QLatin1String("Signature") << QLatin1String("QML id"));
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultsTable->setColumnCount(resultLabels.count());
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->verticalHeader()->hide();
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setHorizontalHeaderLabels(resultLabels);
    connect(m_resultsTable, SIGNAL(itemSelectionChanged()), this, SLOT(onResultsItemSelected()));

    tabWidget->addTab(queryFrame, tr("Query"));
    tabWidget->addTab(flagsFrame, tr("Query Flags"));
    tabWidget->addTab(m_resultsTable, tr("Results"));

    return tabWidget;
}

void QTestBrowserDlg::updateAppContext()
{
    disconnect(m_appContext, SIGNAL(currentIndexChanged(QString)), this, SLOT(onAppContextChanged(QString)));
    m_appContext->clear();
    m_appContext->setCurrentIndex(-1);
    QStringList appIds = m_test->getAppIds();
    int appIndex = appIds.indexOf(m_test->currentAppId());
    foreach (const QString &id, appIds) {
        QString appName;
        QVariantMap appData = m_test->applicationData(id);
        appName = appData["appName"].toString();
        appName.append(QString("(%1)").arg(id));
        m_appContext->addItem(appName, id);
    }
    m_appContext->setCurrentIndex(appIndex);
    connect(m_appContext, SIGNAL(currentIndexChanged(QString)), this, SLOT(onAppContextChanged(QString)));
}

QSize QTestBrowserDlg::sizeHint() const
{
    return QSize(960, 960);
}

void QTestBrowserDlg::contextMenuEvent(QContextMenuEvent *e)
{
    m_methodsAction->setText(QString("View Methods for %1").arg(currentTabSignature()));

    QMenu menu(this);
    menu.addAction(m_refreshAction);
    menu.addSeparator();
    menu.addAction(m_expandAction);
    menu.addAction(m_collapseAction);
    menu.addSeparator();
    menu.addAction(m_methodsAction);
    menu.addSeparator();

    QMenu *subMenu = menu.addMenu(tr("Add Query Condition"));
    subMenu->addAction(QLatin1String("CHILD_OF"));
    subMenu->addAction(QLatin1String("DESCENDANT_OF"));
    subMenu->addAction(QLatin1String("PARENT_OF"));
    subMenu->addAction(QLatin1String("ANCESTOR_OF"));
    connect(subMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuQueryCondition(QAction*)));
    menu.exec(e->globalPos());
}

void QTestBrowserDlg::setSystemTest ( QstConnection *test )
{
    m_test = test;
    connect(m_test, SIGNAL(appFinished(QString)), this, SLOT(onAppFinished(QString)));
    connect(m_test, SIGNAL(appStarted(QString)), this, SLOT(onAppStarted(QString)));
}

void QTestBrowserDlg::setImage( const QPixmap &img )
{
    if (!img.isNull()) {
        m_image->setPixmap(img);
    } else {
        m_image->setText("No image available");
    }

    m_image->adjustSize();
    updateGeometry();
}

void QTestBrowserDlg::setBaseItems( const QVariantMap &item, const QVariantMap &imgItem )
{
    if (item.isEmpty()) {
        m_appId = m_test->currentAppId();
        qDebug() << "current app id" << m_appId;
        m_appId = m_appContext->itemData(m_appContext->currentIndex()).toString();
        m_test->setAppId(m_appId);
        QVariantMap appData = m_test->applicationData(m_appId);
        m_baseItem = appData["appObject"].toMap();
        m_imgItem = m_baseItem;
    } else {
        m_baseItem = item;
        m_imgItem = imgItem;
        m_appId = item[QstConnection::appIdProperty].toString();
    }
    refresh();
}

void QTestBrowserDlg::setObjectMap ( const QVariantMap &map )
{
    m_tree->setCursor(Qt::WaitCursor);
    m_map = map;
    m_geometry = m_map["geometry"].toMap();
    m_children = m_map["children"].toMap();
    m_window = m_map["window"].toMap();
    QString sig = signature(m_baseItem);

    m_tree->clear();
    m_treeMap.clear();

    addItemToTree(sig);
    m_tree->expandAll();
    m_tree->resizeColumnToContents(0);
    m_tree->setCursor(Qt::ArrowCursor);
}

void QTestBrowserDlg::closeWindow()
{
    QDialog::accept();
    deleteLater();
}

void QTestBrowserDlg::onImageClicked( const QPoint& point )
{
    QPoint p = point + m_origin;
    QString selectedItem;
    int selectedArea=-1;

    foreach (const QString &item, m_window.keys()) {
        if (m_window[item] == m_currentWindowSignature) {
            QRect rect = m_geometry[item].toRect();
            if (rect.contains(p) && m_map["visible"].toMap()[item].toBool()) {
                if (selectedArea == -1 || selectedArea > abs(rect.width() * rect.height())) {
                    selectedItem = item;
                    selectedArea = abs(rect.width() * rect.height());
                }
            }
        }
    }
    selectItem(selectedItem);
}

void QTestBrowserDlg::selectItem( const QString &item )
{
    QTreeWidgetItem *treeItem = m_treeMap[item];
    if (treeItem && treeItem != m_tree->currentItem() && !treeItem->isDisabled()) {
        m_tree->collapseAll();
        m_tree->setCurrentItem(treeItem);
    }
}

void QTestBrowserDlg::onItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *prev)
{
    Q_UNUSED(prev);
    if (!item || !m_test) {
        return;
    }

    QString itemSignature = item->text(1);
    QString windowSignature = m_window[itemSignature].toString();
    if (windowSignature != m_currentWindowSignature) {
        m_currentWindowSignature = windowSignature;
        m_imgItem = widgetSpec(windowSignature);
        refreshImage();
    }

    QRect rect;
    rect = m_geometry[item->text(1)].toRect();
    rect.moveTopLeft(rect.topLeft() - m_origin);
    m_image->setSelectedArea(rect);

    QVariantMap propertyMap =
        m_test->getProperties(widgetSpec(itemSignature));

    updatePropertyTable(m_itemPropertyTable, propertyMap);
    m_tabItemSignatures.replace(0, itemSignature);
    m_propertyTab->setCurrentIndex(0);
    m_propertyTab->setTabEnabled(0, true);
    m_propertyTab->setTabToolTip(0, QString("Properties: %1").arg(itemSignature));
}

void QTestBrowserDlg::onAppContextChanged(const QString &context)
{
    Q_UNUSED(context);
    setBaseItems();
}

void QTestBrowserDlg::initTable(QTableWidget *table, const QStringList &labels)
{
    table->setColumnCount(labels.count());
    table->verticalHeader()->hide();
    table->horizontalHeader()->setStretchLastSection(true);
    table->setHorizontalHeaderLabels(labels);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
}

void QTestBrowserDlg::initPropertyTable(QTableWidget *table)
{
    static QStringList propertyHeaders(QStringList()
        << QLatin1String("Property")
        << QLatin1String("Value")
    );

    initTable(table, propertyHeaders);
    connect(table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onPropertyDoubleClicked(QTableWidgetItem*)));
    connect(table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPropertyContextMenu(QPoint)));
}

void QTestBrowserDlg::initMethodsTable(QTableWidget *table)
{
    static QStringList methodsHeaders(QStringList()
        << QLatin1String("Method Type")
        << QLatin1String("Return Type")
        << QLatin1String("Method Signature")
    );

    initTable(table, methodsHeaders);
    connect(table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onMethodsContextMenu(QPoint)));
}

void QTestBrowserDlg::updatePropertyTable(QTableWidget *table, const QVariantMap &propertyMap)
{
    QVariantMap previousValues = table->property("_qttPrevMap").toMap();

    table->clearContents();
    table->setRowCount(propertyMap.size());
    table->clearSpans();

    int row = 0;
    foreach (const QString &propertyName, propertyMap.keys()) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(propertyName);
        nameItem->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 0, nameItem);
        QVariant value = propertyMap[propertyName];

        if (value.type() == QVariant::List) {
            QVariantList list = value.toList();
            if (list.size() > 0) {
                if (list.size() > 1) {
                    table->setRowCount(table->rowCount() + list.size() -1);
                    table->setSpan(row, 0, list.size(), 1);
                }
                foreach (const QVariant &item, list) {
                    QTableWidgetItem *valueItem = valueToTableItem(item);
                    valueItem->setFlags(Qt::ItemIsEnabled);
                    table->setItem(row++, 1, valueItem);
                }
                continue;
            }
        }

        QTableWidgetItem *valueItem = valueToTableItem(value, previousValues.value(propertyName));
        valueItem->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 1, valueItem);
        row++;
    }

    table->resizeRowsToContents();
    int size = table->horizontalHeader()->sectionSize(0);
    table->resizeColumnToContents(0);
    if (size > table->horizontalHeader()->sectionSize(0)) {
        table->horizontalHeader()->resizeSection(0, size);
    }
    table->setProperty("_qttPrevMap", propertyMap);
}

void QTestBrowserDlg::updateMethodsTable(QTableWidget *table, const QVariantList &methods)
{
    table->clearContents();
    table->setRowCount(methods.size());
    table->clearSpans();

    int row = 0;
    foreach (const QVariant &method, methods) {
        QVariantMap map = method.toMap();
        int methodType = map["methodType"].toInt();
        QString methodTypeName;
        switch (methodType) {
          case QMetaMethod::Method:
              methodTypeName = QLatin1String("Method");
              break;
          case QMetaMethod::Signal:
              methodTypeName = QLatin1String("Signal");
              break;
          case QMetaMethod::Slot:
              methodTypeName = QLatin1String("Slot");
              break;
          case QMetaMethod::Constructor:
              methodTypeName = QLatin1String("Constructor");
              break;
        }
        QString returnType = map["returnType"].toString();
        QString signature = map["signature"].toString();

        QTableWidgetItem *typeItem = new QTableWidgetItem(methodTypeName);
        typeItem->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 0, typeItem);

        QTableWidgetItem *retItem = new QTableWidgetItem(returnType);
        retItem->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 1, retItem);

        QTableWidgetItem *sigItem = new QTableWidgetItem(signature);
        sigItem->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 2, sigItem);

        row++;
    }

    table->resizeRowsToContents();
    table->resizeColumnsToContents();
    table->sortItems(1);
}

QTableWidgetItem *QTestBrowserDlg::valueToTableItem(const QVariant &value, const QVariant &previous)
{
    QString valueString = valueToString(value);
    QTableWidgetItem *valueItem = new QTableWidgetItem(valueString);
    if (previous.isValid() && valueToString(previous) != valueString) {
        valueItem->setBackground(QBrush("#ffff80"));
    }
    if (value.type() == QVariant::Map) {
        QString sig = signature(value.toMap());
        if (m_treeMap.contains(sig)) {
            valueItem->setForeground(QBrush(Qt::blue));
        } else if (!sig.isEmpty()) {
            valueItem->setForeground(QBrush(Qt::darkGreen));
        }
    } else if (valueString.startsWith(QChar('<'))
        && valueString.endsWith(QChar('>'))) {
        valueItem->setForeground(QBrush(Qt::gray));
        return valueItem;
    }
    valueItem->setData(Qt::UserRole, value);
    return valueItem;
}

QString QTestBrowserDlg::valueToString(const QVariant &value)
{
    QString valueString;
    if (value.canConvert(QVariant::String)) {
        valueString = value.toString();
    } else if (value.type() == QVariant::List) {
        QVariantList list = value.toList();
        if (list.length() == 0) {
            valueString = QLatin1String("<Empty List>");
        } else {
            QStringList itemStrings;
            foreach (const QVariant& item, list) {
                itemStrings << valueToString(item);
            }
            valueString = itemStrings.join(QLatin1String(", "));
        }
    } else if (value.type() == QVariant::Map) {
        QVariantMap map = value.toMap();
        valueString = signature(map);
        if (valueString.isEmpty()) {
            if (map.contains(QstConnection::enumProperty)) {
                valueString = QString("%1 (%2)").arg(map[QstConnection::valueProperty].toInt()).arg(map[QstConnection::enumProperty].toString());
            } else {
                valueString = QLatin1String("<QVariantMap>");
            }
        }
    } else if (value.type() == QVariant::PointF) {
        QRectF rectf = value.toRectF();
        valueString = QString("QPointF (%1,%2)").arg(rectf.x()).arg(rectf.y());
    } else if (value.type() == QVariant::RectF) {
        QRectF rectf = value.toRectF();
        valueString = QString("QRectF (%1,%2 %3x%4)").arg(rectf.x()).arg(rectf.y()).arg(rectf.width()).arg(rectf.height());
    } else if (value.type() == QVariant::Size) {
        QSize size = value.toSize();
        valueString = QString("QSize (%1x%2)").arg(size.width()).arg(size.height());
    } else if (value.type() == QVariant::SizeF) {
        QSizeF size = value.toSizeF();
        valueString = QString("QSizeF (%1x%2)").arg(size.width()).arg(size.height());
    } else {
        valueString = QString("<%1>").arg(value.typeName());
    }
    return valueString;
}

void QTestBrowserDlg::addItemToTree(const QString &item, QTreeWidgetItem *parent)
{
    QString geoString;
    if (m_geometry.contains(item)) {
        QRect rect = m_geometry[item].toRect();
        geoString = QString("%1,%2 %3x%4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
    }
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(parent, QStringList() <<
                                m_map["type"].toMap()[item].toString() <<
                                item <<
                                m_map["qmlid"].toMap()[item].toString() <<
                                m_map["objectName"].toMap()[item].toString() <<
                                geoString <<
                                m_map["text"].toMap()[item].toString()
    );
    if (!m_map["visible"].toMap()[item].toBool()) {
        treeItem->setForeground(0, QBrush(Qt::gray));
    }
    m_tree->addTopLevelItem(treeItem);
    m_treeMap[item] = treeItem;

    foreach (const QString &child, m_children[item].toStringList()) {
        addItemToTree(child, treeItem);
    }
}

void QTestBrowserDlg::addQueryRow(const QString &property, const QVariant &value)
{
    static QStringList relationshipProperties(QStringList()
      << QLatin1String("ANCESTOR_OF") << QLatin1String("CHILD_OF") << QLatin1String("DESCENDANT_OF") << QLatin1String("PARENT_OF"));

    static QStringList qmlProperties(QStringList()
      << QLatin1String("QMLID") << QLatin1String("QMLINHERITS") << QLatin1String("QMLTYPE"));

    static QStringList defaultProperties(QStringList()
      << QLatin1String("CLASSNAME") << QLatin1String("INHERITS") << QLatin1String("LABEL") << QLatin1String("PROPERTY"));

    int row = m_queryTable->rowCount()-1;
    m_queryTable->insertRow(row);

    QTableWidgetItem *checkItem = new QTableWidgetItem(QString());
    checkItem->setCheckState(Qt::Checked);
    m_queryTable->setItem(row, 0, checkItem);

    QComboBox *propertyCombo = new QComboBox();
    propertyCombo->addItems(defaultProperties);
    propertyCombo->insertSeparator(99);
    propertyCombo->addItems(relationshipProperties);
    propertyCombo->insertSeparator(99);
    propertyCombo->addItems(qmlProperties);
    propertyCombo->setEditable(true);
    propertyCombo->lineEdit()->setText(property);
    propertyCombo->setMaxVisibleItems(propertyCombo->count()+1);
    m_queryTable->setCellWidget(row, 1, propertyCombo);

    QTableWidgetItem *valueItem = new QTableWidgetItem();
    if (value.canConvert(QVariant::String)) {
        valueItem->setText(value.toString());
        valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    } else {
        valueItem->setText(valueToString(value));
        valueItem->setData(Qt::UserRole, value);
        valueItem->setFlags(Qt::NoItemFlags);
    }

    m_queryTable->setItem(row, 2, valueItem);
    m_queryTable->resizeRowsToContents();
    m_queryTable->scrollToItem(checkItem, QAbstractItemView::PositionAtTop);
}

QVariantMap QTestBrowserDlg::queryMap() const
{
    QVariantMap map;
    for (int i=0; i<m_queryTable->rowCount()-1;) {
        QComboBox *propertyCombo = qobject_cast<QComboBox*>(m_queryTable->cellWidget(i, 1));
        QString property;
        if (propertyCombo) {
            property = propertyCombo->currentText();
        }
        QTableWidgetItem *valueItem = m_queryTable->item(i, 2);
        QVariant value = valueItem->data(Qt::UserRole);
        if (!value.isValid()) {
            value = valueItem->text();
        }
        if (property.isEmpty() && value.isNull()) {
            m_queryTable->removeRow(i);
            continue;
        }

        if (property.isEmpty()) {
            QMessageBox::warning(const_cast<QTestBrowserDlg*>(this), tr("Invalid Query"), QLatin1String("An empty property is not permitted"));
            return QVariantMap();
        }

        if (map.contains(property)) {
            QMessageBox::warning(const_cast<QTestBrowserDlg*>(this), tr("Invalid Query"), QString("There are duplicate instances of property '%1'").arg(property));
            return QVariantMap();
        }

        map[property] = value;
        i++;
    }

    if (map.isEmpty()) {
        QMessageBox::warning(const_cast<QTestBrowserDlg*>(this), tr("Invalid Query"), QLatin1String("No properties were specified !"));
    }

    return map;
}

void QTestBrowserDlg::doQuery()
{
    m_resultsTable->clearContents();
    m_resultsTable->setRowCount(0);

    QVariantMap map = queryMap();
    if (map.isEmpty()) {
        return;
    }

    int flags=0;
    if (m_flagInvisible->isChecked()) flags |= QstConnection::ScanInvisible;
    if (m_flagDisabled->isChecked()) flags |= QstConnection::ScanDisabledItems;
    if (m_flagIgnoreScan->isChecked()) flags |= QstConnection::IgnoreScanned;

    QString currentAppId = m_test->currentAppId();
    if (currentAppId != m_appId) {
        m_test->setAppId(m_appId);
    }
    m_matches = m_test->findByProperties(map, flags);
    if (currentAppId != m_appId) {
        m_test->setAppId(currentAppId);
    }

    m_resultsTable->setRowCount(m_matches.count());
    int row=0;
    QVariantMap idMap = m_map["qmlid"].toMap();
    foreach (const QString &match, m_matches) {
        QTableWidgetItem *item = new QTableWidgetItem(match);
        m_resultsTable->setItem(row, 0, item);
        QTableWidgetItem *idItem = new QTableWidgetItem(idMap[match].toString());
        m_resultsTable->setItem(row, 1, idItem);
        row++;
    }
    m_resultsTable->resizeRowsToContents();
    m_resultsTable->resizeColumnsToContents();
    m_queryTab->setTabText(2, QString("Results (%1)").arg(m_matches.count()));
    m_queryTab->setCurrentIndex(2);
}

void QTestBrowserDlg::clearQuery()
{
    for (int i=0; i<m_queryTable->rowCount()-1;) {
        m_queryTable->removeRow(i);
    }
}

void QTestBrowserDlg::copyQuery()
{
    QVariantMap map = queryMap();
    if (map.isEmpty()) {
        return;
    }
//     QApplication::clipboard()->setText(m_test->objectToString(map));
}

void QTestBrowserDlg::expandAll()
{
    m_tree->expandAll();
    m_tree->scrollToItem(m_tree->currentItem());
}

void QTestBrowserDlg::collapseAll()
{

    m_tree->collapseAll();
    m_tree->scrollToItem(m_tree->currentItem());
}

void QTestBrowserDlg::refresh()
{
    setObjectMap(m_test->getObjects(m_baseItem));
    refreshImage();
}

void QTestBrowserDlg::refreshImage()
{
    m_tree->setEnabled(false);
    QImage img(m_test->grabImage(m_imgItem));
    m_origin = m_geometry[m_imgItem["_signature_"].toString()].toRect().topLeft();
    setImage(QPixmap::fromImage(img));
    m_tree->setEnabled(true);
}

void QTestBrowserDlg::showMatch(int index)
{
    selectItem(m_matches[index]);
}

void QTestBrowserDlg::viewMethods()
{
    QString currentItem = currentTabSignature();
    QVariantList methods =
        m_test->getMethods(widgetSpec(currentItem));

    QTableWidget *methodsTable = new QTableWidget();
    initMethodsTable(methodsTable);
    updateMethodsTable(methodsTable, methods);
    int newTabIndex = m_propertyTab->addTab(methodsTable, QString("Methods: %1").arg(currentItem));
    m_tabItemSignatures.append(currentItem);
    m_propertyTab->setCurrentIndex(newTabIndex);
}

void QTestBrowserDlg::onPropertyDoubleClicked(QTableWidgetItem *item)
{
    QString itemText = item->text();
    if (m_treeMap.contains(itemText)) {
        selectItem(itemText);
    } else {
        QVariant value = item->data(Qt::UserRole);
        if (value.type() == QVariant::Map) {
            QVariantMap map = value.toMap();
            if (map.contains(QstConnection::signatureProperty)) {
                openPropertyTab(map[QstConnection::signatureProperty].toString());
            }
        }
    }
}

void QTestBrowserDlg::openPropertyTab(const QString &signature)
{
    QVariantMap propertyMap =
        m_test->getProperties(widgetSpec(signature));

    QTableWidget *propertyTable = new QTableWidget();
    initPropertyTable(propertyTable);
    updatePropertyTable(propertyTable, propertyMap);
    int newTabIndex = m_propertyTab->addTab(propertyTable, QString("Properties: %1").arg(signature));
    m_tabItemSignatures.append(signature);
    m_propertyTab->setCurrentIndex(newTabIndex);

}

void QTestBrowserDlg::onTabCloseRequested(int tabIndex)
{
    if (tabIndex > 0) {
        QWidget *w = m_propertyTab->widget(tabIndex);
        m_propertyTab->removeTab(tabIndex);
        delete w;
        m_tabItemSignatures.removeAt(tabIndex);
    }
}

QString QTestBrowserDlg::currentTabSignature() const
{
    return m_tabItemSignatures.at(m_propertyTab->currentIndex());
}

void QTestBrowserDlg::onPropertyContextMenu(const QPoint &pos)
{
    QTableWidget *propertyTable = qobject_cast<QTableWidget*>(m_propertyTab->currentWidget());
    if (propertyTable) {
        m_contextTableItem = propertyTable->itemAt(pos);
        QMenu menu(this);

        m_methodsAction->setText(QString("View Methods for %1").arg(currentTabSignature()));
        menu.addAction(m_propertyRefreshAction);
        menu.addSeparator();
        menu.addAction(m_methodsAction);

        if (tableItemIsWidgetIdentifier(m_contextTableItem)) {
            m_propertyMethodsAction->setText(QString("View Methods for %1").arg(m_contextTableItem->text()));
            m_propertyPropertiesAction->setText(QString("View Properties for %1").arg(m_contextTableItem->text()));
            menu.addSeparator();
            menu.addAction(m_propertyMethodsAction);
            menu.addAction(m_propertyPropertiesAction);
        }

        // Add as Query Condition, only if item is not part of a list
        // and the value is valid
        if (m_contextTableItem
            && propertyTable->rowSpan(m_contextTableItem->row(), 0) == 1
            && propertyTable->item(m_contextTableItem->row(), 1)->data(Qt::UserRole).isValid()) {
            menu.addSeparator();
            menu.addAction(m_propertyQueryAction);
        }
        menu.addSeparator();
        menu.addAction(m_propertyCopyAction);
        menu.addAction(m_propertyCopyTableAction);
        menu.exec(propertyTable->mapToGlobal(pos));
        m_contextTableItem = 0;
    }
}

void QTestBrowserDlg::onMethodsContextMenu(const QPoint &pos)
{
    QTableWidget *propertyTable = qobject_cast<QTableWidget*>(m_propertyTab->currentWidget());
    if (propertyTable) {
        m_contextTableItem = propertyTable->itemAt(pos);
        QMenu menu(this);
        menu.addAction(m_propertyCopyAction);
        menu.addAction(m_propertyCopyTableAction);
        menu.exec(propertyTable->mapToGlobal(pos));
        m_contextTableItem = 0;
    }
}

void QTestBrowserDlg::refreshProperties()
{
    QTableWidget *propertyTable = qobject_cast<QTableWidget*>(m_propertyTab->currentWidget());
    if (propertyTable) {
        propertyTable->setCursor(Qt::WaitCursor);
        QVariantMap propertyMap =
            m_test->getProperties(widgetSpec(currentTabSignature()));
        updatePropertyTable(propertyTable, propertyMap);
        propertyTable->setCursor(Qt::ArrowCursor);
    }
}

void QTestBrowserDlg::viewPropertyMethods()
{
    if (m_contextTableItem) {
        QString signature = m_contextTableItem->text();
        QVariantList methods =
            m_test->getMethods(widgetSpec(signature));

        QTableWidget *methodsTable = new QTableWidget();
        initMethodsTable(methodsTable);
        updateMethodsTable(methodsTable, methods);
        int newTabIndex = m_propertyTab->addTab(methodsTable, QString("Methods: %1").arg(signature));
        m_tabItemSignatures.append(signature);
        m_propertyTab->setCurrentIndex(newTabIndex);
    }
}

void QTestBrowserDlg::viewPropertyProperties()
{
    if (m_contextTableItem) {
        QString signature = m_contextTableItem->text();
        openPropertyTab(signature);
    }
}

void QTestBrowserDlg::addPropertyAsQueryCondition()
{
    if (m_contextTableItem) {
        int row = m_contextTableItem->row();
        QTableWidget *table = m_contextTableItem->tableWidget();
        QTableWidgetItem *propertyItem = table->item(row, 0);
        QTableWidgetItem *valueItem = table->item(row, 1);
        QVariant value = valueItem->data(Qt::UserRole);
        if (value.type() == QVariant::Map) {
            QVariantMap map = value.toMap();
            if (map.contains("_enum_"))
                value = map.value("_enum_");
            else if (map.contains("_signature_"))
                value = map.value("_signature_");
            else
                return;
        }
        addQueryRow(propertyItem->text(), value);
    }
    m_queryTab->setCurrentIndex(0);
}

void QTestBrowserDlg::copyTableItemValue()
{
    if (m_contextTableItem) {
        QApplication::clipboard()->setText(m_contextTableItem->text());
    }
}

void QTestBrowserDlg::copyTable()
{
    QTableWidget *propertyTable = qobject_cast<QTableWidget*>(m_propertyTab->currentWidget());
    QString text;
    for (int row = 0; row < propertyTable->rowCount(); ++row) {
        for (int col = 0; col < propertyTable->columnCount(); ++col) {
            QString cellText;
            QTableWidgetItem *item = propertyTable->item(row, col);
            if (item) {
                cellText = propertyTable->item(row, col)->text();
            }
            text += QString("%1%2").arg(col ? "\t" : "").arg(cellText);
        }
        text += QLatin1String("\n");
    }
    QApplication::clipboard()->setText(text);
}

bool QTestBrowserDlg::tableItemIsWidgetIdentifier(const QTableWidgetItem *item)
{
    if (!item) return false;
    QVariant value = item->data(Qt::UserRole);
    return value.type() == QVariant::Map
            && value.toMap().contains(QstConnection::signatureProperty);
}

QVariantMap QTestBrowserDlg::widgetSpec(const QString &signature)
{
    return m_test->widgetFromSignature(signature, m_appId);
}

void QTestBrowserDlg::onConnectionClosed()
{
    QMessageBox::warning(this, tr("Connection Lost"), tr("Connection to the test system has been lost."));
}

void QTestBrowserDlg::onQueryItemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0 && item->checkState() == Qt::Unchecked) {
        m_queryTable->removeRow(item->row());
    }
}

void QTestBrowserDlg::onQueryContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    menu.addAction(m_queryClearAction);
    menu.addAction(m_queryCopyAction);

    menu.exec(m_queryTable->mapToGlobal(pos));
}

void QTestBrowserDlg::onResultsItemSelected()
{
    selectItem(m_resultsTable->item(m_resultsTable->currentRow(), 0)->text());
}

void QTestBrowserDlg::onMenuQueryCondition(QAction *action)
{
    addQueryRow(action->text(), currentTabSignature());
    m_queryTab->setCurrentIndex(0);
}

void QTestBrowserDlg::onAppFinished(const QString& appId)
{
    updateAppContext();
    if (appId == m_appId) {
        QMessageBox::warning(this, tr("Application Closed"), tr("The application was closed."));
        setBaseItems();
    }
}

void QTestBrowserDlg::onAppStarted(const QString& appId)
{
    Q_UNUSED(appId);
    updateAppContext();
}

bool QTestBrowserDlg::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_appContext && event->type() == QEvent::MouseButtonPress) {
        for (int i=0; i<m_appContext->count(); i++) {
            if (m_appContext->itemText(i).isEmpty()) {
                QVariantMap appData = m_test->applicationData(m_appContext->itemData(i).toString());
                m_appContext->setItemText(i, appData["appId"].toString());
            }
        }
        return false;
    } else {
        return QDialog::eventFilter(obj, event);
    }
}
