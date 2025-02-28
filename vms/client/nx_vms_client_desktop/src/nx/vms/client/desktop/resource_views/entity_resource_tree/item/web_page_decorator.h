// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/vms/client/desktop/resource_views/entity_item_model/item/abstract_item.h>

namespace nx::vms::client::desktop::entity_resource_tree {

class WebPageDecorator: public entity_item_model::AbstractItem
{
public:
    WebPageDecorator(entity_item_model::AbstractItemPtr sourceItem, bool hasAdminPermissions);

    virtual QVariant data(int role) const override;
    virtual Qt::ItemFlags flags() const override;

private:
    entity_item_model::AbstractItemPtr m_sourceItem;
    bool m_hasAdminPermissions = false;
};

} // namespace nx::vms::client::desktop::entity_resource_tree
