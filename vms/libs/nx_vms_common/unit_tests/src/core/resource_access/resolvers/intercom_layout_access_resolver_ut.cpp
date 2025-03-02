// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include <iostream>
#include <memory>

#include <core/resource/camera_resource.h>
#include <core/resource/layout_resource.h>
#include <core/resource_access/global_permissions_watcher.h>
#include <core/resource_access/resolvers/own_resource_access_resolver.h>
#include <core/resource_access/resolvers/intercom_layout_access_resolver.h>

#include "private/resource_access_resolver_test_fixture.h"
#include "../access_rights_types_testing.h"

namespace nx::core::access {
namespace test {

using namespace nx::vms::api;

class IntercomLayoutAccessResolverTest: public ResourceAccessResolverTestFixture
{
public:
    virtual AbstractResourceAccessResolver* createResolver() const override
    {
        const auto ownResolver = new OwnResourceAccessResolver(
            manager.get(), systemContext()->globalPermissionsWatcher(), /*parent*/ manager.get());
        return new IntercomLayoutAccessResolver(ownResolver, resourcePool());
    }
};

TEST_F(IntercomLayoutAccessResolverTest, noAccess)
{
    const auto intercom = addIntercomCamera();
    const auto layout = addIntercomLayout(intercom);
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, intercom), kNoAccessRights);
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, layout), kNoAccessRights);
}

TEST_F(IntercomLayoutAccessResolverTest, noDirectAccessToIntercomLayout)
{
    const auto intercom = addIntercomCamera();
    const auto layout = addIntercomLayout(intercom);

    manager->setOwnResourceAccessMap(kTestSubjectId, {{layout->getId(), kFullAccessRights}});
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, intercom), kNoAccessRights);
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, layout), kNoAccessRights);

    manager->setOwnResourceAccessMap(kTestSubjectId,
        {{layout->getId(), kFullAccessRights}, {intercom->getId(), kViewAccessRights}});
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, intercom), kViewAccessRights);
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, layout), kViewAccessRights);
}

TEST_F(IntercomLayoutAccessResolverTest, accessToLayoutViaIntercom)
{
    const auto intercom = addIntercomCamera();
    const auto layout = addIntercomLayout(intercom);
    manager->setOwnResourceAccessMap(kTestSubjectId, {{intercom->getId(), kFullAccessRights}});
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, intercom), kFullAccessRights);
    EXPECT_EQ(resolver->accessRights(kTestSubjectId, layout), kFullAccessRights);
}

TEST_F(IntercomLayoutAccessResolverTest, accessDetailsViaIntercom)
{
    const auto intercom = addIntercomCamera();
    const auto layout = addIntercomLayout(intercom);
    manager->setOwnResourceAccessMap(kTestSubjectId, {{intercom->getId(), kFullAccessRights}});

    EXPECT_EQ(resolver->accessDetails(kTestSubjectId, layout, AccessRight::view),
        ResourceAccessDetails({{kTestSubjectId, {intercom}}}));
}

TEST_F(IntercomLayoutAccessResolverTest, notificationSignals)
{
    // Add intercom.
    const auto intercom = addIntercomCamera();
    const auto layout = addIntercomLayout(intercom);
    manager->setOwnResourceAccessMap(kTestSubjectId, {{intercom->getId(), kFullAccessRights}});
    NX_ASSERT_TEST_SUBJECT_CHANGED();

    // Change intercom access rights.
    manager->setOwnResourceAccessMap(kTestSubjectId, {{intercom->getId(), kViewAccessRights}});
    NX_ASSERT_TEST_SUBJECT_CHANGED();

    // Remove.
    manager->removeSubjects({kTestSubjectId});
    NX_ASSERT_TEST_SUBJECT_CHANGED();

    // Clear.
    manager->clear();
    NX_ASSERT_SIGNAL(resourceAccessReset);
    NX_ASSERT_NO_SIGNAL(resourceAccessChanged);
}

} // namespace test
} // namespace nx::core::access
