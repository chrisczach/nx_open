// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "show_on_alarm_layout_action.h"

#include "../action_builder_fields/event_devices_field.h"
#include "../action_builder_fields/event_id_field.h"
#include "../action_builder_fields/flag_field.h"
#include "../action_builder_fields/target_device_field.h"
#include "../action_builder_fields/target_user_field.h"
#include "../utils/event_details.h"
#include "../utils/field.h"
#include "../utils/type.h"

namespace nx::vms::rules {

const ItemDescriptor& ShowOnAlarmLayoutAction::manifest()
{
    static const auto kDescriptor = ItemDescriptor{
        .id = utils::type<ShowOnAlarmLayoutAction>(),
        .displayName = tr("Show on Alarm Layout"),
        .fields = {
            makeFieldDescriptor<TargetDeviceField>(utils::kDeviceIdsFieldName, "Cameras"),
            utils::makeIntervalFieldDescriptor("Interval of action"),
            utils::makePlaybackFieldDescriptor(tr("Playback Time")),
            makeFieldDescriptor<TargetUserField>(utils::kUsersFieldName, tr("For")),
            makeFieldDescriptor<FlagField>("forceOpen", tr("Force Alarm Layout opening")),

            makeFieldDescriptor<EventIdField>("id", "Event ID"),
            makeFieldDescriptor<EventDevicesField>("eventDeviceIds", "Event devices"),
            utils::makeTextFormatterFieldDescriptor("caption", tr("Alarm: %1").arg("{@EventCaption}")),
            utils::makeTextFormatterFieldDescriptor("description", "{@EventDescription}"),
            utils::makeTextFormatterFieldDescriptor("tooltip", "{@EventTooltip}"),
            utils::makeExtractDetailFieldDescriptor("sourceName", utils::kSourceNameDetailName),
        }
    };
    return kDescriptor;
}

} // namespace nx::vms::rules
