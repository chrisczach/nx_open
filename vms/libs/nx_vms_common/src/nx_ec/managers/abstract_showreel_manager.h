// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <QtCore/QObject>

#include <nx/utils/async_handler_executor.h>
#include <nx/utils/uuid.h>
#include <nx/vms/api/data/showreel_data.h>

#include "../ec_api_common.h"

namespace ec2 {

class NX_VMS_COMMON_API AbstractShowreelNotificationManager: public QObject
{
    Q_OBJECT

signals:
    void addedOrUpdated(const nx::vms::api::ShowreelData& tour, ec2::NotificationSource source);
    void removed(const QnUuid& id);
};

/*!
\note All methods are asynchronous if other not specified
*/
class NX_VMS_COMMON_API AbstractShowreelManager
{
public:
    virtual ~AbstractShowreelManager() = default;

    virtual int getShowreels(
        Handler<nx::vms::api::ShowreelDataList> handler,
        nx::utils::AsyncHandlerExecutor handlerExecutor = {}) = 0;

    ErrorCode getShowreelsSync(nx::vms::api::ShowreelDataList* outDataList);

    virtual int save(
        const nx::vms::api::ShowreelData& data,
        Handler<> handler,
        nx::utils::AsyncHandlerExecutor handlerExecutor = {}) = 0;

    ErrorCode saveSync(const nx::vms::api::ShowreelData& data);

    virtual int remove(
        const QnUuid& tourId,
        Handler<> handler,
        nx::utils::AsyncHandlerExecutor handlerExecutor = {}) = 0;

    ErrorCode removeSync(const QnUuid& tourId);
};

} // namespace ec2
