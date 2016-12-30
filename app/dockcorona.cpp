/*
 * Copyright 2014  Bhushan Shah <bhush94@gmail.com>
 * Copyright 2014 Marco Martin <notmart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "dockcorona.h"
#include "dockview.h"
#include "packageplugins/shell/dockpackage.h"

#include <QAction>
#include <QScreen>
#include <QDebug>

#include <Plasma>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <KActionCollection>
#include <KPluginMetaData>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

namespace Latte {

DockCorona::DockCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    KPackage::Package package(new DockPackage(this));
    
    if (!package.isValid()) {
        qWarning() << staticMetaObject.className()
                   << "the package" << package.metadata().rawData() << "is invalid!";
        return;
    } else {
        qDebug() << staticMetaObject.className()
                 << "the package" << package.metadata().rawData() << "is valid!";
    }
    
    setKPackage(package);
    qmlRegisterTypes();
    
    connect(this, &Corona::containmentAdded, this, &DockCorona::addDock);
    
    loadLayout();
    
    /*QAction *addDock = actions()->add<QAction>(QStringLiteral("add dock"));
    connect(addDock, &QAction::triggered, this, &NowDockCorona::loadDefaultLayout);
    addDock->setText(i18n("Add New Dock"));
    addDock->setAutoRepeat(true);
    addDock->setStatusTip(tr("Adds a new dock in the environment"));
    addDock->setVisible(true);
    addDock->setEnabled(true);
    
    addDock->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));
    addDock->setData(Plasma::Types::ControlAction);
    addDock->setShortcut(QKeySequence(QStringLiteral("alt+d, l")));
    addDock->setShortcutContext(Qt::ApplicationShortcut);*/
}

DockCorona::~DockCorona()
{
    for (auto c : m_containments)
        c->deleteLater();
        
    qDebug() << "deleted" << this;
}

int DockCorona::numScreens() const
{
    return qGuiApp->screens().count();
}

QRect DockCorona::screenGeometry(int id) const
{
    const auto screens = qGuiApp->screens();
    
    if (id >= 0 && id < screens.count()) {
        return screens[id]->geometry();
    }
    
    return qGuiApp->primaryScreen()->geometry();
}

QRegion DockCorona::availableScreenRegion(int id) const
{
    const auto screens = qGuiApp->screens();
    
    if (id >= 0 && id < screens.count()) {
        return screens[id]->geometry();
    }
    
    return qGuiApp->primaryScreen()->availableGeometry();
}

QRect DockCorona::availableScreenRect(int id) const
{
    const auto screens = qGuiApp->screens();
    
    if (id >= 0 && id < screens.count()) {
        return screens[id]->availableGeometry();
    }
    
    return qGuiApp->primaryScreen()->availableGeometry();
}

int DockCorona::primaryScreenId() const
{
    const auto screens = qGuiApp->screens();
    
    int id = -1;
    
    for (int i = 0; i < screens.size(); ++i) {
        auto *scr = screens.at(i);
        
        if (scr == qGuiApp->primaryScreen()) {
            id = i;
            break;
        }
    }
    
    return id;
}

QList<Plasma::Types::Location> DockCorona::freeEdges(int screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                 Types::TopEdge, Types::RightEdge};
                                 
    //when screen=-1 is passed then the primaryScreenid is used
    int fixedScreen = (screen == -1) ? primaryScreenId() : screen;
    
    for (const DockView *cont : m_containments) {
        if (cont && cont->containment()->screen() == fixedScreen)
            edges.removeOne(cont->location());
    }
    
    return edges;
}

int DockCorona::screenForContainment(const Plasma::Containment *containment) const
{
    for (auto *view : m_containments) {
        if (view && view->containment() && view->containment()->id() == containment->id())
            if (view->screen())
                return qGuiApp->screens().indexOf(view->screen());
    }
    
    return -1;
}

void DockCorona::addDock(Plasma::Containment *containment)
{
    if (!containment || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }
    
    // the system tray is a containment that behaves as an applet
    // so a dockview shouldnt be created for it
    KPluginMetaData metadata = containment->kPackage().metadata();
    
    if (metadata.pluginId() == "org.kde.plasma.private.systemtray") {
        return;
    }
    
    foreach (DockView *dock, m_containments) {
        if (dock->containment() == containment) {
            return;
        }
    }
    
    qDebug() << "Adding dock for container...";
    
    auto dockView = new DockView(this);
    dockView->init();
    dockView->setContainment(containment);
    dockView->show();
    
    m_containments.push_back(dockView);
}

void DockCorona::loadDefaultLayout()
{

    qDebug() << "loading default layout";
    //! Settting mutable for create a containment
    setImmutability(Plasma::Types::Mutable);
    
    QVariantList args;
    auto defaultContainment = createContainmentDelayed("org.kde.latte.containment", args);
    
    defaultContainment->setContainmentType(Plasma::Types::PanelContainment);
    defaultContainment->init();
    
    if (!defaultContainment || !defaultContainment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }
    
    auto config = defaultContainment->config();
    defaultContainment->restore(config);
    
    QList<Plasma::Types::Location> edges = freeEdges(defaultContainment->screen());
    
    if (edges.count() > 0) {
        defaultContainment->setLocation(edges.at(0));
    } else {
        defaultContainment->setLocation(Plasma::Types::BottomEdge);
    }
    
    defaultContainment->updateConstraints(Plasma::Types::StartupCompletedConstraint);
    defaultContainment->save(config);
    requestConfigSync();
    defaultContainment->flushPendingConstraintsEvents();
    emit containmentAdded(defaultContainment);
    emit containmentCreated(defaultContainment);
    
    addDock(defaultContainment);
    
    defaultContainment->createApplet(QStringLiteral("org.kde.latte.plasmoid"));
    defaultContainment->createApplet(QStringLiteral("org.kde.plasma.analogclock"));
}

inline void DockCorona::qmlRegisterTypes() const
{
    constexpr auto uri = "org.kde.latte.shell";
    constexpr auto vMajor = 0;
    constexpr auto vMinor = 2;
    
//    qmlRegisterUncreatableType<Candil::Dock>(uri, vMajor, vMinor, "Dock", "class Dock uncreatable");
//    qmlRegisterUncreatableType<Candil::VisibilityManager>(uri, vMajor, vMinor, "VisibilityManager", "class VisibilityManager uncreatable");
//    qmlRegisterUncreatableType<NowDockView>(uri, vMajor, vMinor, "DockView", "class DockView uncreatable");
    qmlRegisterType<QScreen>();
}

}
