// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "generic_event.h"

#include "../event_filter_fields/dummy_field.h"
#include "../event_filter_fields/keywords_field.h"
#include "../utils/event_details.h"
#include "../utils/field.h"
#include "../utils/type.h"

namespace nx::vms::rules {

GenericEvent::GenericEvent(
    std::chrono::microseconds timestamp,
    State state,
    const QString& caption,
    const QString& description,
    const QString& source,
    QnUuid serverId,
    const QnUuidList& deviceIds)
    :
    base_type(timestamp, caption, description),
    m_source(source),
    m_serverId(serverId),
    m_deviceIds(deviceIds)
{
    setState(state);
}

QString GenericEvent::resourceKey() const
{
    return {};
}

QVariantMap GenericEvent::details(common::SystemContext* context) const
{
    auto result = base_type::details(context);

    result.remove(utils::kSourceIdDetailName);
    utils::insertIfNotEmpty(result, utils::kExtraCaptionDetailName, extraCaption());
    utils::insertIfNotEmpty(result, utils::kExtendedCaptionDetailName, extendedCaption());
    utils::insertIfNotEmpty(result, utils::kDetailingDetailName, description());
    utils::insertIfNotEmpty(result, utils::kSourceNameDetailName, source());
    result.insert(utils::kEmailTemplatePathDetailName, manifest().emailTemplatePath);
    utils::insertLevel(result, nx::vms::event::Level::common);
    utils::insertIcon(result, icon());
    utils::insertClientAction(result, ClientAction::previewCameraOnTime);

    return result;
}

QString GenericEvent::extendedCaption() const
{
    return m_source.isEmpty()
        ? tr("Generic Event")
        : tr("Generic Event at %1").arg(m_source);
}

Icon GenericEvent::icon() const
{
    return deviceIds().isEmpty() ? Icon::alert : Icon::camera;
}

const ItemDescriptor& GenericEvent::manifest()
{
    static const QString kKeywordFieldDescription = tr("Keywords separated by space");
    static const auto kDescriptor = ItemDescriptor{
        .id = utils::type<GenericEvent>(),
        .displayName = tr("Generic Event"),
        .flags = {ItemFlag::instant, ItemFlag::prolonged},
        .fields = {
            utils::makeStateFieldDescriptor(tr("State")),
            makeFieldDescriptor<KeywordsField>("source", tr("Source"), kKeywordFieldDescription),
            makeFieldDescriptor<KeywordsField>("caption", tr("Caption"), kKeywordFieldDescription),
            makeFieldDescriptor<KeywordsField>("description",
                tr("Description"),
                kKeywordFieldDescription),

            makeFieldDescriptor<DummyField>(utils::kDeviceIdsFieldName, utils::kDeviceIdsFieldName)
        },
        .permissions = {
            .resourcePermissions = {
                {utils::kDeviceIdsFieldName, {Qn::ViewContentPermission, Qn::ViewFootagePermission}}
            }
        },
        .emailTemplatePath = ":/email_templates/generic_event.mustache"
    };
    return kDescriptor;
}

} // namespace nx::vms::rules
