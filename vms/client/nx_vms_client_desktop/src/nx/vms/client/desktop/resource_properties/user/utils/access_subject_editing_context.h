// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <QtCore/QObject>

#include <core/resource/resource_fwd.h>
#include <core/resource_access/resource_access_details.h>
#include <core/resource_access/resource_access_map.h>
#include <nx/utils/impl_ptr.h>

namespace nx::vms::common { class SystemContext; }
namespace nx::core::access { class SubjectHierarchy; }

namespace nx::vms::client::desktop {

class AccessSubjectEditingContext: public QObject
{
    Q_OBJECT
    using base_type = QObject;

    Q_PROPERTY(nx::vms::common::ResourceFilter accessibleResourcesFilter
        READ accessibleResourcesFilter NOTIFY resourceAccessChanged)

    Q_PROPERTY(nx::vms::api::AccessRights availableAccessRights
        READ availableAccessRights NOTIFY resourceAccessChanged)

public:
    explicit AccessSubjectEditingContext(
        nx::vms::common::SystemContext* systemContext, QObject* parent = nullptr);

    virtual ~AccessSubjectEditingContext() override;

    nx::vms::common::SystemContext* systemContext() const;

    /** A current subject being edited. */
    QnUuid currentSubjectId() const;
    void setCurrentSubjectId(const QnUuid& subjectId);

    nx::core::access::ResourceAccessMap ownResourceAccessMap() const;

    bool hasOwnAccessRight(
        const QnUuid& resourceOrGroupId, nx::vms::api::AccessRight accessRight) const;

    /** Overrides current subject access rights. */
    void setOwnResourceAccessMap(const nx::core::access::ResourceAccessMap& resourceAccessMap);

    /** Reverts current subject access rights editing. */
    void resetOwnResourceAccessMap();

    /** Returns full resolved access rights for a specified resource. */
    Q_INVOKABLE nx::vms::api::AccessRights accessRights(const QnResourcePtr& resource) const;

    /**
     * Returns all ways in which the specified subject gains specified access right to
     * the specified resource, directly and indirectly.
     */
    nx::core::access::ResourceAccessDetails accessDetails(
        const QnResourcePtr& resource,
        nx::vms::api::AccessRight accessRight) const;

    nx::vms::common::ResourceFilter accessibleResourcesFilter() const;
    nx::vms::api::AccessRights availableAccessRights() const;

    /** Edit current subject relations with other subjects. */
    void setRelations(const QSet<QnUuid>& parents, const QSet<QnUuid>& members);

    /** Revert current subject relations with other subjects to original values. */
    void resetRelations();

    /** Returns subject hierarchy being edited. */
    const nx::core::access::SubjectHierarchy* subjectHierarchy() const;

    /** Returns whether current subject access rights or inheritance were changed. */
    bool hasChanges() const;

    /** Reverts any changes. */
    void revert();

    nx::vms::api::GlobalPermissions globalPermissions() const;

    QSet<QnUuid> globalPermissionSource(nx::vms::api::GlobalPermission perm) const;

    static QnUuid specialResourceGroupFor(const QnResourcePtr& resource);

    static bool isRelevant(
        nx::vms::api::SpecialResourceGroup group, nx::vms::api::AccessRight accessRight);

signals:
    void subjectChanged();
    void hierarchyChanged();
    void currentSubjectRemoved();
    void resourceAccessChanged();
    void resourceGroupsChanged(const QSet<QnUuid>& resourceGroupIds);

private:
    class Private;
    nx::utils::ImplPtr<Private> d;
};

} // namespace nx::vms::client::desktop
