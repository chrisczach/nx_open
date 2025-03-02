// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <core/resource/resource_fwd.h>
#include <nx/utils/impl_ptr.h>
#include <nx/vms/client/desktop/common/models/filter_proxy_model.h>
#include <nx/vms/client/desktop/resource_views/data/resource_tree_globals.h>
#include <nx/vms/client/desktop/system_context.h>

class QnResource;
class QnWorkbenchContext;

namespace nx::vms::client::desktop {

class ResourceSelectionModelAdapter: public ScopedModelOperations<FilterProxyModel>
{
    Q_OBJECT

    Q_PROPERTY(nx::vms::client::desktop::SystemContext* context
        READ context WRITE setContext NOTIFY contextChanged)
    Q_PROPERTY(nx::vms::client::desktop::ResourceTree::ResourceFilters resourceTypes
        READ resourceTypes WRITE setResourceTypes NOTIFY resourceTypesChanged)
    Q_PROPERTY(nx::vms::client::desktop::ResourceTree::ResourceSelection selectionMode
        READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(nx::vms::common::ResourceFilter resourceFilter READ resourceFilter
        WRITE setResourceFilter NOTIFY resourceFilterChanged)
    Q_PROPERTY(bool extraInfoRequired READ isExtraInfoRequired NOTIFY extraInfoRequiredChanged)

    using base_type = ScopedModelOperations<FilterProxyModel>;

public:
    explicit ResourceSelectionModelAdapter(QObject* parent = nullptr);
    virtual ~ResourceSelectionModelAdapter() override;

    SystemContext* context() const;
    void setContext(SystemContext* context);

    ResourceTree::ResourceFilters resourceTypes() const;
    void setResourceTypes(ResourceTree::ResourceFilters value);

    ResourceTree::ResourceSelection selectionMode() const;
    void setSelectionMode(ResourceTree::ResourceSelection value);

    QString filterText() const;
    void setFilterText(const QString& value);

    nx::vms::common::ResourceFilter resourceFilter() const;
    void setResourceFilter(nx::vms::common::ResourceFilter value);

    bool isExtraInfoRequired() const;

    Q_INVOKABLE bool isExtraInfoForced(QnResource* resource) const;

    QSet<QnResourcePtr> selectedResources() const;
    QSet<QnUuid> selectedResourceIds() const;
    QModelIndex resourceIndex(const QnResourcePtr& resource) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    static void registerQmlType();

signals:
    void contextChanged();
    void filterTextChanged();
    void resourceFilterChanged();
    void resourceTypesChanged();
    void selectionModeChanged();
    void extraInfoRequiredChanged();
    void selectedResourcesChanged();

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    struct Private;
    nx::utils::ImplPtr<Private> d;
};

} // namespace nx::vms::client::desktop
