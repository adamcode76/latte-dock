/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VIEWSCONTROLLER_H
#define VIEWSCONTROLLER_H

// local
#include <coretypes.h>
#include "viewsmodel.h"
#include "../../lattecorona.h"
#include "../../data/viewdata.h"
#include "../../data/viewstable.h"

// Qt
#include <QAbstractItemModel>
#include <QHash>
#include <QItemSelection>
#include <QList>
#include <QMetaObject>
#include <QSortFilterProxyModel>
#include <QTableView>

// KDE
#include <KMessageWidget>

namespace Latte {
class CentralLayout;
class Corona;
class ViewsDialog;

namespace Settings {
namespace Handler {
class ViewsHandler;
}
namespace View {
class ViewsTableView;
}
}
}

namespace Latte {
namespace Settings {
namespace Controller {

class Views : public QObject
{
    Q_OBJECT

public:
    explicit Views(Settings::Handler::ViewsHandler *parent);
    ~Views();

    QAbstractItemModel *proxyModel() const;
    QAbstractItemModel *baseModel() const;
    QTableView *view() const;

    bool hasChangedData() const;

    int viewsForRemovalCount() const;

    void sortByColumn(int column, Qt::SortOrder order);

    bool hasSelectedView() const;
    const Data::ViewsTable selectedViewsCurrentData() const;

    const Latte::Data::View appendViewFromViewTemplate(const Data::View &view);

    void selectRow(const QString &id);

    //! actions
    void reset();
    void save();

public slots:
    void copySelectedViews();
    void cutSelectedViews();
    void duplicateSelectedViews();
    void pasteSelectedViews();
    void removeSelectedViews();

signals:
    void dataChanged();

private:
    void init();

    bool hasValidOriginView(const Data::View &view);
    CentralLayout *originLayout(const Data::View &view);

    int rowForId(QString id) const;
    QString uniqueViewName(QString name);
    QString visibleViewName(const QString &id) const;

    Data::ViewsTable selectedViewsForClipboard();

    //! errors/warnings
    void messagesForErrorsWarnings(const Latte::CentralLayout *centralLayout);
    void messageForErrorAppletsWithSameId(const Data::Error &error);
    void messageForErrorOrphanedParentAppletOfSubContainment(const Data::Error &error);
    void messageForWarningOrphanedSubContainments(const Data::Warning &warning);
    void messageForWarningAppletAndContainmentWithSameId(const Data::Warning &warning);

private slots:
    void loadConfig();
    void saveConfig();
    void storeColumnWidths();
    void applyColumnWidths();

    void onCurrentLayoutChanged();
    void onSelectionsChanged();

    void showDefaultPersistentErrorWarningInlineMessage(const QString &messageText,
                                                        const KMessageWidget::MessageType &messageType,
                                                        QList<QAction *> extraActions = QList<QAction *>(),
                                                        const bool &showOpenLayoutAction = true);

    void updateDoubledMoveDestinationRows();

private:
    Settings::Handler::ViewsHandler *m_handler{nullptr};

    Settings::View::ViewsTableView *m_view{nullptr};

    int m_debugSaveCall{0};

    //! current active layout signals/slots
    QList<QMetaObject::Connection> m_currentLayoutConnections;

    //! layoutsView ui settings
    int m_viewSortColumn{Model::Views::SCREENCOLUMN};
    Qt::SortOrder m_viewSortOrder;
    QStringList m_viewColumnWidths;

    KConfigGroup m_storage;

    //! context menu actions for docks panels
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_duplicateAction;
    QAction *m_pasteAction;

    //! current data
    Model::Views *m_model{nullptr};
    QSortFilterProxyModel *m_proxyModel{nullptr};
};

}
}
}

#endif

