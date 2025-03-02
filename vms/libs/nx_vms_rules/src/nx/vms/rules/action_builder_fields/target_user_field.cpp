// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "target_user_field.h"

#include <core/resource/user_resource.h>
#include <core/resource_access/resource_access_subject_hierarchy.h>
#include <core/resource_management/resource_pool.h>
#include <core/resource_management/user_roles_manager.h>
#include <nx/utils/qset.h>
#include <nx/vms/common/system_context.h>

namespace nx::vms::rules {

TargetUserField::TargetUserField(common::SystemContext* context):
    common::SystemContextAware(context)
{
}

QnUserResourceSet TargetUserField::users() const
{
    QnUserResourceSet result;

    if (acceptAll())
    {
        result = nx::utils::toQSet(resourcePool()->getResources<QnUserResource>());
    }
    else
    {
        QnUserResourceList users;
        QnUuidList roles;
        systemContext()->userRolesManager()->usersAndRoles(ids(), users, roles);

        result = nx::utils::toQSet(users);

        const auto groupUsers = systemContext()->accessSubjectHierarchy()->usersInGroups(
            nx::utils::toQSet(roles));

        for (const auto& user: groupUsers)
            result.insert(user);
    }

    erase_if(result, [](const auto& user) { return !user || !user->isEnabled(); });

    return result;
}

} // namespace nx::vms::rules
