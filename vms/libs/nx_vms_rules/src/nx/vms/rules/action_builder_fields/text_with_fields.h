// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/vms/common/system_context_aware.h>

#include "../action_builder_field.h"

namespace nx::vms::rules {

 /** Perform string formatting using event data values. */
class NX_VMS_RULES_API TextWithFields:
    public ActionBuilderField,
    public common::SystemContextAware
{
    Q_OBJECT
    Q_CLASSINFO("metatype", "nx.actions.fields.textWithFields")

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit TextWithFields(common::SystemContext* context);

    virtual QVariant build(const AggregatedEventPtr& eventAggregator) const override;

    QString text() const;
    void setText(const QString& text);

    static const QStringList& availableFunctions();

signals:
    void textChanged();

private:
    QString m_text;

    enum class FieldType
    {
        Text,
        Substitution
    };

    QList<FieldType> m_types;
    QList<QString> m_values;
};

/** Same as base class but hidden from user. */
class NX_VMS_RULES_API TextFormatter: public TextWithFields
{
    Q_OBJECT
    Q_CLASSINFO("metatype", "nx.actions.fields.textFormatter")

public:
    using TextWithFields::TextWithFields;
};

} // namespace nx::vms::rules
