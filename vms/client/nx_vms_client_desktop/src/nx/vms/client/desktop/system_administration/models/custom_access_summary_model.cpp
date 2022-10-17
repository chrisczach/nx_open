// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "custom_access_summary_model.h"

#include <algorithm>

#include <QtQml/QtQml>


namespace nx::vms::client::desktop {

namespace {

struct PermissionFlagInfo
{
    std::variant<api::AccessRight, api::GlobalPermission> flag;
    QString text;
};

static const std::vector<PermissionFlagInfo> kPermissionSummary = {
    {api::AccessRight::viewLive, CustomAccessSummaryModel::tr("View live")},
    {api::AccessRight::listenToAudio, CustomAccessSummaryModel::tr("Access audio from camera")},
    {api::AccessRight::viewArchive, CustomAccessSummaryModel::tr("View video archive")},
    {api::AccessRight::exportArchive, CustomAccessSummaryModel::tr("Export video archive")},
    {api::AccessRight::viewBookmarks, CustomAccessSummaryModel::tr("View bookmarks")},
    {api::AccessRight::manageBookmarks, CustomAccessSummaryModel::tr("Modify bookmarks")},
    {api::AccessRight::userInput, CustomAccessSummaryModel::tr("User input")},
    {api::AccessRight::editSettings, CustomAccessSummaryModel::tr("Edit camera settings")},
    {api::AccessRight::managePtz, CustomAccessSummaryModel::tr("Edit PTZ presets and tours")},
    {api::GlobalPermission::viewLogs, CustomAccessSummaryModel::tr("View event log")},
};

} // namespace

CustomAccessSummaryModel::CustomAccessSummaryModel(QObject* parent):
    base_type(parent)
{
}

CustomAccessSummaryModel::~CustomAccessSummaryModel()
{
}

bool CustomAccessSummaryModel::testFlag(
    const std::variant<api::AccessRight, api::GlobalPermission>& flag) const
{
    if (const auto* accessRight = std::get_if<api::AccessRight>(&flag))
        return m_customRights.testFlag(*accessRight);
    else if (const auto* globalPermission = std::get_if<api::GlobalPermission>(&flag))
        return m_ownGlobalPermissions.testFlag(*globalPermission);

    return false;
}

QVariant CustomAccessSummaryModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int) kPermissionSummary.size())
        return {};

    switch (role)
    {
        case Qt::DisplayRole:
            return kPermissionSummary.at(index.row()).text;

        case Qt::CheckStateRole:
        {
            return testFlag(kPermissionSummary.at(index.row()).flag)
                ? Qt::Checked
                : Qt::Unchecked;
        }
        default:
            return {};
    }
}

int CustomAccessSummaryModel::rowCount(const QModelIndex&) const
{
    return m_hasDisplayableItems ? kPermissionSummary.size() : 0;
}

QHash<int, QByteArray> CustomAccessSummaryModel::roleNames() const
{
    auto names = base_type::roleNames();

    names.insert(Qt::CheckStateRole, "checkState");

    return names;
}

void CustomAccessSummaryModel::setOwnGlobalPermissions(nx::vms::api::GlobalPermissions permissions)
{
    if (m_ownGlobalPermissions == permissions)
        return;

    m_ownGlobalPermissions = permissions;
    updateInfo();
}

void CustomAccessSummaryModel::setOwnSharedResources(
    const nx::core::access::ResourceAccessMap& resources)
{
    api::AccessRights customRights = {};

    for (const auto& accessRights: resources.values())
        customRights |= accessRights;

    if (customRights == m_customRights)
        return;

    m_customRights = customRights;
    updateInfo();
}

void CustomAccessSummaryModel::updateInfo()
{
    bool hasDisplayableItems = std::any_of(kPermissionSummary.begin(), kPermissionSummary.end(),
        [this](const auto& it)
        {
            return testFlag(it.flag);
        });

    if (hasDisplayableItems != m_hasDisplayableItems)
    {
        if (hasDisplayableItems)
        {
            beginInsertRows({}, 0, kPermissionSummary.size() - 1);
            m_hasDisplayableItems = hasDisplayableItems;
            endInsertRows();
        }
        else
        {
            beginRemoveRows({}, 0, kPermissionSummary.size() - 1);
            m_hasDisplayableItems = hasDisplayableItems;
            endRemoveRows();
        }
        return;
    }

    if (!m_hasDisplayableItems)
        return;

    emit dataChanged(
        this->index(0, 0),
        this->index(kPermissionSummary.size() - 1, 0));
}

void CustomAccessSummaryModel::registerQmlTypes()
{
    qmlRegisterType<CustomAccessSummaryModel>(
        "nx.vms.client.desktop", 1, 0, "CustomAccessSummaryModel");
}

} // namespace nx::vms::client::desktop
