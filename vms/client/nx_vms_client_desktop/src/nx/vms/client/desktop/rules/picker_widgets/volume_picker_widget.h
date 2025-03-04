// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

#include <nx/vms/rules/action_builder_fields/volume_field.h>

#include "picker_widget.h"

namespace nx::vms::client::desktop::rules {

class VolumePicker: public FieldPickerWidget<vms::rules::VolumeField>
{
    Q_OBJECT

public:
    VolumePicker(QnWorkbenchContext* context, CommonParamsWidget* parent);

private:
    QSlider* m_volumeSlider = nullptr;
    QPushButton* m_testPushButton = nullptr;
    float m_audioDeviceCachedVolume = 0.;

    void updateUi() override;

    void onVolumeChanged();
    void onTestButtonClicked();
    Q_INVOKABLE void onTextSaid();
};

} // namespace nx::vms::client::desktop::rules
