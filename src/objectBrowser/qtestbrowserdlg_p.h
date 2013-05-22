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


#ifndef QTESTBROWSERDLG_P_H
#define QTESTBROWSERDLG_P_H

#include <QDialog>
#include <QLabel>
#include <QVariant>
#include <QVariantMap>
#include <QtSystemTest/qstglobal.h>

class QstConnection;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPushButton;
class QMouseEvent;
class QPaintEvent;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
class QTableWidgetItem;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QTabWidget;
class QCheckBox;
QT_END_NAMESPACE

class TestImageViewer : public QLabel
{
    Q_OBJECT

public:
    TestImageViewer( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~TestImageViewer();
    void setSelectedArea(const QRect& rect);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

signals:
    void clicked(const QPoint&);

private:
    QRect m_selectedArea;
};

class QSYSTEMTEST_EXPORT QTestBrowserDlg : public QDialog
{
    Q_OBJECT

public:
    QTestBrowserDlg(QstConnection*, const QVariantMap& = QVariantMap(), const QVariantMap& = QVariantMap());
    ~QTestBrowserDlg();

    virtual QSize sizeHint() const;
    virtual void contextMenuEvent(QContextMenuEvent *e);
    void setSystemTest ( QstConnection *test );
    void setImage( const QPixmap& );
    void setBaseItems( const QVariantMap& = QVariantMap(), const QVariantMap& = QVariantMap() );
    void setObjectMap ( const QVariantMap& );

public slots:
    void closeWindow();
    void onImageClicked(const QPoint&);
    void onItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *prev);
    void onAppContextChanged(const QString& context);
    void addQueryRow(const QString &property = QString(), const QVariant &value = QString());
    void doQuery();
    void clearQuery();
    void copyQuery();
    void expandAll();
    void collapseAll();
    void refresh();
    void refreshImage();
    void showMatch(int);
    void viewMethods();
    void selectItem(const QString& item);
    void onPropertyDoubleClicked(QTableWidgetItem *item);
    void onTabCloseRequested(int);
    void onPropertyContextMenu(const QPoint&);
    void onMethodsContextMenu(const QPoint&);
    void refreshProperties();
    void viewPropertyMethods();
    void viewPropertyProperties();
    void addPropertyAsQueryCondition();
    void copyTableItemValue();
    void copyTable();
    void onConnectionClosed();
    void onQueryItemChanged(QTableWidgetItem *item);
    void onQueryContextMenu(const QPoint&);
    void onResultsItemSelected();
    void onMenuQueryCondition(QAction *action);
    void onAppFinished(const QString&);
    void onAppStarted(const QString&);

protected:
    bool eventFilter( QObject *obj, QEvent *event);

private:
    void createActions();
    void createWidgets();
    QTabWidget *createQueryTab();
    void updateAppContext();

    void addItemToTree(const QString &item, QTreeWidgetItem *parent = 0);
    void initTable(QTableWidget *table, const QStringList&);
    void initPropertyTable(QTableWidget *table);
    void initMethodsTable(QTableWidget *table);
    void updatePropertyTable(QTableWidget *table, const QVariantMap &propertyMap);
    void updateMethodsTable(QTableWidget *table, const QVariantList &methods);
    QTableWidgetItem *valueToTableItem(const QVariant &value, const QVariant &previous = QVariant());
    QString valueToString(const QVariant &value);
    QString currentTabSignature() const;
    bool tableItemIsWidgetIdentifier(const QTableWidgetItem *item);
    QVariantMap widgetSpec(const QString &signature);
    void openPropertyTab(const QString &signature);
    QVariantMap queryMap() const;

    QVariantMap m_baseItem;
    QVariantMap m_imgItem;
    QString m_appId;
    QString m_visibleTabItem;
    QPoint m_origin;
    QVariantMap m_map;
    QVariantMap m_geometry;
    QVariantMap m_children;
    QVariantMap m_window;
    QMap<QString,QTreeWidgetItem*> m_treeMap;
    QStringList m_matches;
    QStringList m_tabItemSignatures;
    QString m_currentWindowSignature;

    QstConnection *m_test;
    TestImageViewer *m_image;
    QComboBox *m_appContext;
    QTreeWidget *m_tree;
    QTabWidget *m_queryTab;
    QTabWidget *m_propertyTab;
    QTableWidget *m_itemPropertyTable;
    QTableWidget *m_queryTable;
    QTableWidget *m_resultsTable;
    QTableWidgetItem *m_contextTableItem;

    QCheckBox *m_flagInvisible;
    QCheckBox *m_flagDisabled;
    QCheckBox *m_flagIgnoreScan;

    QAction *m_refreshAction;
    QAction *m_expandAction;
    QAction *m_collapseAction;
    QAction *m_methodsAction;

    // Table context menu actions
    QAction *m_propertyRefreshAction;
    QAction *m_propertyMethodsAction;
    QAction *m_propertyPropertiesAction;
    QAction *m_propertyQueryAction;
    QAction *m_propertyCopyAction;
    QAction *m_propertyCopyTableAction;

    // Query actions
    QAction *m_queryClearAction;
    QAction *m_queryCopyAction;
};

#endif
