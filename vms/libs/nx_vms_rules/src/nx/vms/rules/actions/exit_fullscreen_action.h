// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include "../basic_action.h"
#include "../data_macros.h"

namespace nx::vms::rules {

class NX_VMS_RULES_API ExitFullscreenAction: public nx::vms::rules::BasicAction
{
    Q_OBJECT
    Q_CLASSINFO("type", "nx.actions.exitFullscreen")

    FIELD(QnUuidSet, layoutIds, setLayoutIds)

public:
    static const ItemDescriptor& manifest();
};

} // namespace nx::vms::rules
