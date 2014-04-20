/*
  outboundconnectionmodel.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "outboundconnectionsmodel.h"

#include <core/probe.h>

#include <QDebug>
#include <private/qobject_p.h>

using namespace GammaRay;

OutboundConnectionsModel::OutboundConnectionsModel(QObject* parent):
  AbstractConnectionsModel(parent)
{
}

OutboundConnectionsModel::~OutboundConnectionsModel()
{
}

void OutboundConnectionsModel::setObject(QObject* object)
{
  beginResetModel();
  m_connections.clear();
  m_object = object;

  if (!object) {
    return;
    endResetModel();
  }

  QObjectPrivate *d = QObjectPrivate::get(object);
  if (d->connectionLists) {
    // HACK: the declaration of d->connectionsLists is not accessible for us...
    const QVector<QObjectPrivate::ConnectionList> *cl = reinterpret_cast<QVector<QObjectPrivate::ConnectionList>*>(d->connectionLists);
    for (int signalIndex = 0; signalIndex < cl->count(); ++signalIndex) {
      const QObjectPrivate::Connection *c = cl->at(signalIndex).first;
      while (c) {
        if (!c->receiver || Probe::instance()->filterObject(c->receiver)) {
          c = c->nextConnectionList;
          continue;
        }

        Connection conn;
        conn.endpoint = c->receiver;
        conn.signalIndex = signalIndex;
        conn.slotIndex = c->method();
        conn.type = c->connectionType;
        c = c->nextConnectionList;
        m_connections.push_back(conn);
      }
    }
  }

  endResetModel();
}

QVariant OutboundConnectionsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || !m_object)
    return QVariant();

  if (role == Qt::DisplayRole) {
    const Connection &conn = m_connections.at(index.row());
    switch (index.column()) {
      case 0:
        return displayString(m_object, conn.signalIndex);
      case 1:
        return displayString(conn.endpoint);
      case 2:
        return displayString(conn.endpoint, conn.slotIndex);
    }
  }

  return AbstractConnectionsModel::data(index, role);
}

QVariant OutboundConnectionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
      case 0: return tr("Signal");
      case 1: return tr("Receiver");
      case 2: return tr("Slot");
    }
  }
  return AbstractConnectionsModel::headerData(section, orientation, role);
}