// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <functional>
#include <set>

#include <QtCore/QRectF>

#include <nx/vms/client/core/event_search/models/abstract_async_search_list_model.h>

class QnWorkbenchContext;

namespace nx::analytics::db { struct ObjectTrack; }

namespace nx::vms::client::core { class TextFilterSetup; }

namespace nx::vms::client::desktop {

class AnalyticsSearchListModel: public core::AbstractAsyncSearchListModel
{
    Q_OBJECT
    using base_type = AbstractAsyncSearchListModel;

public:
    explicit AnalyticsSearchListModel(QnWorkbenchContext* context, QObject* parent = nullptr);
    virtual ~AnalyticsSearchListModel() override = default;

    QRectF filterRect() const;
    void setFilterRect(const QRectF& relativeRect);

    virtual core::TextFilterSetup* textFilter() const override;

    QnUuid selectedEngine() const;
    void setSelectedEngine(const QnUuid& value);

    QString selectedObjectType() const;
    void setSelectedObjectType(const QString& value);
    const std::set<QString>& relevantObjectTypes() const;

    QStringList attributeFilters() const;
    void setAttributeFilters(const QStringList& value);

    QString combinedTextFilter() const; // Free text filter combined with attribute filters.

    // Metadata newer than timestamp returned by this callback will be deferred.
    using LiveTimestampGetter = std::function<
        std::chrono::milliseconds(const QnVirtualCameraResourcePtr&)>;
    void setLiveTimestampGetter(LiveTimestampGetter value);

    virtual bool isConstrained() const override;
    virtual bool hasAccessRights() const override;

    enum class LiveProcessingMode
    {
        automaticAdd,
        manualAdd
    };
    Q_ENUM(LiveProcessingMode)

    LiveProcessingMode liveProcessingMode() const;
    void setLiveProcessingMode(LiveProcessingMode value);

    // Methods for `LiveProcessingMode::manualAdd` mode.
    int availableNewTracks() const;
    void commitAvailableNewTracks();
    static constexpr int kUnknownAvailableTrackCount = -1;

signals:
    void pluginActionRequested(const QnUuid& engineId, const QString& actionTypeId,
        const nx::analytics::db::ObjectTrack& track, const QnVirtualCameraResourcePtr& camera,
        QPrivateSignal);

    void filterRectChanged();
    void selectedEngineChanged();
    void selectedObjectTypeChanged();
    void relevantObjectTypesChanged();
    void availableNewTracksChanged();
    void attributeFiltersChanged();
    void combinedTextFilterChanged();

private:
    class Private;
    Private* const d;
};

} // namespace nx::vms::client::desktop
