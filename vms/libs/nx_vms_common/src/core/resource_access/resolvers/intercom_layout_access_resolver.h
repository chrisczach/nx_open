// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include "abstract_resource_access_resolver.h"

#include <nx/utils/impl_ptr.h>

class QnResourcePool;

namespace nx::core::access {

/**
 * This class resolves an indirect access granted by intercom devices to their layouts.
 */
class NX_VMS_COMMON_API IntercomLayoutAccessResolver: public AbstractResourceAccessResolver
{
    Q_OBJECT
    using base_type = AbstractResourceAccessResolver;

public:
    explicit IntercomLayoutAccessResolver(
        AbstractResourceAccessResolver* baseResolver,
        QnResourcePool* resourcePool,
        QObject* parent = nullptr);

    virtual ~IntercomLayoutAccessResolver() override;

    virtual ResourceAccessMap resourceAccessMap(const QnUuid& subjectId) const override;

    virtual nx::vms::api::GlobalPermissions globalPermissions(const QnUuid& subjectId) const override;

    virtual ResourceAccessDetails accessDetails(
        const QnUuid& subjectId,
        const QnResourcePtr& resource,
        nx::vms::api::AccessRight accessRight) const override;

private:
    class Private;
    nx::utils::ImplPtr<Private> d;
};

} // namespace nx::core::access
