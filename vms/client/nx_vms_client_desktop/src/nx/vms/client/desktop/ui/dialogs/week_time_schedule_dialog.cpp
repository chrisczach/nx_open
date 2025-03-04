// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "week_time_schedule_dialog.h"

#include <nx/vms/client/desktop/resource_properties/schedule/schedule_cell_painter.h>
#include <nx/vms/common/utils/schedule.h>
#include <ui/help/help_topic_accessor.h>
#include <ui/help/help_topics.h>
#include <utils/media/bitStream.h>

#include "ui_week_time_schedule_dialog.h"

namespace nx::vms::client::desktop {

WeekTimeScheduleDialog::WeekTimeScheduleDialog(QWidget* parent):
    base_type(parent),
    ui(new Ui::WeekTimeScheduleDialog),
    m_scheduleCellPainter(new nx::vms::client::desktop::ScheduleCellPainter())
{
    ui->setupUi(this);

    ui->gridWidget->setCellPainter(m_scheduleCellPainter.get());
    ui->valueOnButton->setCellPainter(m_scheduleCellPainter.get());
    ui->valueOffButton->setCellPainter(m_scheduleCellPainter.get());

    ui->valueOnButton->setButtonBrush(true);
    ui->valueOffButton->setButtonBrush({});
    ui->gridWidget->setBrush(ui->valueOnButton->buttonBrush());

    setHelpTopic(this, Qn::EventsActions_Schedule_Help);

    connect(ui->valueOnButton,  &QToolButton::clicked, this,
        [this]() { ui->gridWidget->setBrush(ui->valueOnButton->buttonBrush()); });

    connect(ui->valueOffButton, &QToolButton::clicked, this,
        [this]() { ui->gridWidget->setBrush(ui->valueOffButton->buttonBrush()); });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &WeekTimeScheduleDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &WeekTimeScheduleDialog::reject);
}

WeekTimeScheduleDialog::~WeekTimeScheduleDialog()
{
}

QByteArray WeekTimeScheduleDialog::schedule() const
{
    QByteArray buffer(24, 0); // qPower2Floor(24*7/8, 4)
    BitStreamWriter writer;
    writer.setBuffer((uint8_t*) buffer.data(), buffer.size());

    for (const auto dayOfWeek: nx::vms::common::daysOfWeek())
    {
        for (int hour = 0; hour < nx::vms::common::kHoursInDay; ++hour)
            writer.putBit(ui->gridWidget->cellData(dayOfWeek, hour).toBool());
    }

    writer.flushBits();
    buffer.resize(nx::vms::common::kDaysInWeek * nx::vms::common::kHoursInDay / 8);

    return buffer;
}

void WeekTimeScheduleDialog::setSchedule(const QByteArray& schedule)
{
    if (schedule.isEmpty())
    {
        ui->gridWidget->fillGridData(true);
        return;
    }

    BitStreamReader reader((uint8_t*)schedule.data(), schedule.size());

    for (const auto dayOfWeek: nx::vms::common::daysOfWeek())
    {
        for (int hour = 0; hour < nx::vms::common::kHoursInDay; ++hour)
            ui->gridWidget->setCellData(dayOfWeek, hour, reader.getBit());
    }
}

QString WeekTimeScheduleDialog::scheduleTasks() const
{
    return QString::fromUtf8(schedule().toHex());
}

void WeekTimeScheduleDialog::setScheduleTasks(const QString& value)
{
    setSchedule(QByteArray::fromHex(value.toUtf8()));
}

} // namespace nx::vms::client::desktop
