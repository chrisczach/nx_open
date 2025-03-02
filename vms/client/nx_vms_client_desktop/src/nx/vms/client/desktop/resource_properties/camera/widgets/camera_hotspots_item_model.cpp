// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "camera_hotspots_item_model.h"

#include <client/client_globals.h>
#include <core/resource/camera_resource.h>
#include <core/resource_management/resource_pool.h>
#include <nx/vms/client/desktop/style/resource_icon_cache.h>
#include <nx/vms/client/desktop/style/skin.h>
#include <nx/vms/client/desktop/ui/common/color_theme.h>

namespace nx::vms::client::desktop {

CameraHotspotsItemModel::CameraHotspotsItemModel(QnResourcePool* resourcePool, QObject* parent):
    base_type(parent),
    m_resourcePool(resourcePool)
{
    NX_ASSERT(m_resourcePool);
}

CameraHotspotsItemModel::~CameraHotspotsItemModel()
{
}

void CameraHotspotsItemModel::setHotspots(const common::CameraHotspotDataList& hotspots)
{
    if (m_hotspots == hotspots)
        return;

    beginResetModel();
    m_hotspots = hotspots;
    endResetModel();
}

int CameraHotspotsItemModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : Column::ColumnCount;
}

QVariant CameraHotspotsItemModel::data(const QModelIndex& index, int role) const
{
    const auto hotspotData = m_hotspots.at(index.row());
    const auto cameraId = hotspotData.cameraId;
    const auto camera = m_resourcePool->getResourceById<QnVirtualCameraResource>(cameraId);

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case IndexColumn:
                return index.row() + 1;

            case CameraColumn:
                if (camera)
                    return camera->getUserDefinedName();
                else if (!cameraId.isNull())
                    return tr("Camera does not exist");
                else
                    return tr("Select Camera");

            case DeleteButtonColumn:
                return tr("Delete");
        }
    }

    if (role == Qt::DecorationRole)
    {
        switch (index.column())
        {
            case CameraColumn:
                return camera
                    ? qnResIconCache->icon(camera)
                    : qnResIconCache->icon(QnResourceIconCache::Cameras);

            case DeleteButtonColumn:
                return qnSkin->icon("text_buttons/trash.png");
        }
    }

    if (role == Qn::ResourceRole && index.column() == CameraColumn && camera)
        return QVariant::fromValue(camera);

    if (role == Qt::CheckStateRole && index.column() == OrientedCheckBoxColumn)
        return hotspotData.hasDirection() ? Qt::Checked : Qt::Unchecked;

    if (role == HotspotCameraIdRole && index.column() == CameraColumn && !cameraId.isNull())
        return QVariant::fromValue(cameraId);

    if (role == HotspotColorRole && index.column() == ColorPaletteColumn)
        return QColor(hotspotData.accentColorName);

    return {};
}

Qt::ItemFlags CameraHotspotsItemModel::flags(const QModelIndex& index) const
{
    auto result = base_type::flags(index);

    if (index.column() == CameraColumn)
        result.setFlag(Qt::ItemIsEditable);

    return result;
}

QVariant CameraHotspotsItemModel::headerData(
    int section,
    Qt::Orientation orientation,
    int role) const
{
    if (orientation == Qt::Vertical)
        return base_type::headerData(section, orientation, role);

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignLeft;

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
            case IndexColumn:
                return tr("#");

            case Column::CameraColumn:
                return tr("Camera");

            case ColorPaletteColumn:
                return tr("Color");

            case OrientedCheckBoxColumn:
                return tr("Oriented");

            case DeleteButtonColumn:
                return {};

            default:
                NX_ASSERT("Unexpected column");
                return {};
        }
    }

    return base_type::headerData(section, orientation, role);
}

QModelIndex CameraHotspotsItemModel::index(int row, int column, const QModelIndex&) const
{
    return createIndex(row, column);
}

int CameraHotspotsItemModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_hotspots.size();
}

QModelIndex CameraHotspotsItemModel::parent(const QModelIndex&) const
{
    return {};
}

} // namespace nx::vms::client::desktop
