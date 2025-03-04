// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>

#include <nx/vms/rules/action_builder_fields/password_field.h>
#include <nx/vms/rules/action_builder_fields/text_field.h>
#include <nx/vms/rules/event_filter_fields/customizable_text_field.h>
#include <nx/vms/rules/event_filter_fields/text_field.h>

#include "picker_widget.h"
#include "picker_widget_utils.h"

namespace nx::vms::client::desktop::rules {

/**
 * Used for data types that could be represented as a one line text. The actual
 * implementation is depends on the field descriptor set.
 * Has implementations for:
 * - nx.events.fields.customizableText
 * - nx.events.fields.expectedString
 *
 * - nx.actions.fields.text
 * - nx.actions.fields.password
 */

template<typename F>
class OnelineTextPickerWidgetBase: public FieldPickerWidget<F>
{
public:
    OnelineTextPickerWidgetBase(QnWorkbenchContext* context, CommonParamsWidget* parent):
        FieldPickerWidget<F>(context, parent)
    {
        auto contentLayout = new QHBoxLayout;

        m_lineEdit = new QLineEdit;
        m_lineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
        contentLayout->addWidget(m_lineEdit);

        m_contentWidget->setLayout(contentLayout);

        connect(
            m_lineEdit,
            &QLineEdit::textEdited,
            this,
            &OnelineTextPickerWidgetBase<F>::onTextChanged);
    }

protected:
    PICKER_WIDGET_COMMON_USINGS

    QLineEdit* m_lineEdit{nullptr};

    virtual void onTextChanged(const QString& text) = 0;
};

template<typename F>
class OnelineTextPickerWidgetCommon: public OnelineTextPickerWidgetBase<F>
{
public:
    OnelineTextPickerWidgetCommon(QnWorkbenchContext* context, CommonParamsWidget* parent):
        OnelineTextPickerWidgetBase<F>(context, parent)
    {
        if (std::is_same<F, vms::rules::PasswordField>())
            m_lineEdit->setEchoMode(QLineEdit::Password);
    }

protected:
    PICKER_WIDGET_COMMON_USINGS
    using OnelineTextPickerWidgetBase<F>::m_lineEdit;

    void updateUi() override
    {
        const QSignalBlocker blocker{m_lineEdit};
        m_lineEdit->setText(theField()->value());
    }

    void onTextChanged(const QString& text) override
    {
        theField()->setValue(text);
    }
};

using ActionTextPicker = OnelineTextPickerWidgetCommon<vms::rules::ActionTextField>;
using CustomizableTextPicker = OnelineTextPickerWidgetCommon<vms::rules::CustomizableTextField>;
using EventTextPicker = OnelineTextPickerWidgetCommon<vms::rules::EventTextField>;
using PasswordPicker = OnelineTextPickerWidgetCommon<vms::rules::PasswordField>;

} // namespace nx::vms::client::desktop::rules
