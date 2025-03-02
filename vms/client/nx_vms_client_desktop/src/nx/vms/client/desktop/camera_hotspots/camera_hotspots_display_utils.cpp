// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "camera_hotspots_display_utils.h"

#include <cmath>
#include <numbers>

#include <QtGui/QPainter>

#include <nx/utils/log/assert.h>
#include <nx/vms/client/core/utils/geometry.h>
#include <nx/vms/client/desktop/style/helper.h>
#include <nx/vms/client/desktop/style/resource_icon_cache.h>
#include <nx/vms/client/desktop/style/skin.h>
#include <nx/vms/client/desktop/ui/common/color_theme.h>
#include <utils/common/scoped_painter_rollback.h>

namespace {

static constexpr auto kHotspotRadius = 16;
static constexpr auto kHotspotPointerAngle = 90.0;
static constexpr auto kHotspotBoundsOffset = 4;
static constexpr auto kSelectedHotspotOutlineWidth = 2.0;

static constexpr QSize kHotspotItemIconSize =
    {nx::style::Metrics::kDefaultIconSize, nx::style::Metrics::kDefaultIconSize};

static constexpr auto kHotspotBodyOpacity = 0.4;
static constexpr auto kHoveredHotspotBodyOpacity = 0.6;
static constexpr auto kNoCameraIconOpacity = 0.4;

static constexpr auto kHotspotFontSize = 16;

} // namespace

namespace nx::vms::client::desktop {
namespace camera_hotspots {

using Geometry = nx::vms::client::core::Geometry;
using CameraHotspotData = nx::vms::common::CameraHotspotData;

QPointF hotspotOrigin(const CameraHotspotData& hotspot, const QRectF& rect)
{
    const auto origin = Geometry::subPoint(rect, hotspot.pos);
    return Geometry::bounded(origin, Geometry::eroded(rect, kHotspotRadius + kHotspotBoundsOffset));
}

QPointF hotspotOrigin(const QPointF& hotspotRelativePos, const QRectF& rect)
{
    const auto origin = Geometry::subPoint(rect, hotspotRelativePos);
    return Geometry::bounded(origin, Geometry::eroded(rect, kHotspotRadius + kHotspotBoundsOffset));
}

void setHotspotPositionFromPointInRect(
    const QRectF& sourceRect,
    const QPointF& sourcePoint,
    nx::vms::common::CameraHotspotData& outHotspot)
{
    const auto sourceRectPoint = sourcePoint - sourceRect.topLeft();
    outHotspot.pos = Geometry::bounded(
        Geometry::cwiseDiv(sourceRectPoint, sourceRect.size()),
        QRectF(0, 0, 1, 1));
}

QPainterPath makeHotspotOutline(const QPointF& origin, const QPointF& direction)
{
    QPainterPath hotspotRoundOutline;
    hotspotRoundOutline.addEllipse(origin, kHotspotRadius, kHotspotRadius);

    if (direction.isNull())
        return hotspotRoundOutline;

    const auto radiusDirection = Geometry::normalized(direction) * kHotspotRadius;
    const auto hotspotPointerAngleRad = kHotspotPointerAngle * std::numbers::pi / 180.0;

    const auto pointerTipPoint = origin + radiusDirection / std::sin(hotspotPointerAngleRad * 0.5);

    const auto pointerCcwTangentPoint = origin +
        Geometry::rotated(radiusDirection, QPointF(), -90.0 + kHotspotPointerAngle * 0.5);

    const auto pointerCwTangentPoint = origin +
        Geometry::rotated(radiusDirection, QPointF(), 90.0 - kHotspotPointerAngle * 0.5);

    QPainterPath hotspotPointerOutline;
    hotspotPointerOutline.moveTo(pointerTipPoint);
    hotspotPointerOutline.lineTo(pointerCcwTangentPoint);
    hotspotPointerOutline.lineTo(pointerCwTangentPoint);
    hotspotPointerOutline.lineTo(pointerTipPoint);

    return hotspotRoundOutline.united(hotspotPointerOutline);
}

QPainterPath makeHotspotOutline(
    const nx::vms::common::CameraHotspotData& hotspot,
    const QRectF& rect)
{
    const auto origin = hotspotOrigin(hotspot, rect);
    if (qFuzzyIsNull(Geometry::length(hotspot.direction)))
        return makeHotspotOutline(origin);

    const auto absoluteDirection = Geometry::cwiseMul(hotspot.direction, rect.size());
    return makeHotspotOutline(origin, absoluteDirection);
}

void paintHotspot(
    QPainter* painter,
    const common::CameraHotspotData& hotspot,
    const QPointF& origin,
    const CameraHotspotDisplayOption& option)
{
    using namespace nx::vms::client::core;

    static const auto kInvalidColor = colorTheme()->color("camera.hotspots.invalid");

    // Paint hotspot body.
    const auto hotspotOutline = makeHotspotOutline(origin, hotspot.direction);

    const QBrush brush(
        option.cameraState == CameraHotspotDisplayOption::CameraState::invalid
            ? kInvalidColor
            : QColor(hotspot.accentColorName));

    double hotspotBodyOpacity = 0.0;
    switch (option.state)
    {
        case CameraHotspotDisplayOption::State::none:
            hotspotBodyOpacity = kHotspotBodyOpacity;
            break;

        case CameraHotspotDisplayOption::State::hovered:
        case CameraHotspotDisplayOption::State::selected:
            hotspotBodyOpacity = kHoveredHotspotBodyOpacity;
            break;

        case CameraHotspotDisplayOption::State::disabled:
            hotspotBodyOpacity = kHotspotBodyOpacity * nx::style::Hints::kDisabledItemOpacity;
            break;

        default:
            NX_ASSERT(false, "Unexpected hotspot display state");
            break;
    }

    {
        QnScopedPainterOpacityRollback opacityRollback(painter, hotspotBodyOpacity);
        painter->fillPath(hotspotOutline, brush);
    }

    // Paint selected item outline.
    if (option.state == CameraHotspotDisplayOption::State::selected)
    {
        QPen selectedOutlinePen(brush, kSelectedHotspotOutlineWidth);
        painter->strokePath(hotspotOutline, selectedOutlinePen);
    }

    // Paint decoration which is icon or text.
    if (option.decoration.canConvert<QString>())
    {
        static const auto kTextColor = colorTheme()->color("camera.hotspots.textColor");

        double textOpacity = 1.0;
        if (option.state == CameraHotspotDisplayOption::State::disabled)
            textOpacity = nx::style::Hints::kDisabledItemOpacity;

        QnScopedPainterOpacityRollback opacityRollback(painter, textOpacity);

        QFont font;
        font.setPixelSize(kHotspotFontSize);
        QnScopedPainterFontRollback fontRollback(painter, font);
        QnScopedPainterPenRollback penRollback(painter, kTextColor);

        QSize size(kHotspotRadius * 2, kHotspotRadius * 2);
        QRect rect(origin.toPoint() - QPoint(kHotspotRadius, kHotspotRadius), size);

        QTextOption textOption;
        textOption.setAlignment(Qt::AlignCenter);
        painter->drawText(rect, option.decoration.toString(), textOption);
    }
    else if (option.decoration.typeId() == QMetaType::QIcon)
    {
        double iconOpacity = 1.0;
        if (option.cameraState == CameraHotspotDisplayOption::CameraState::noCamera)
            iconOpacity = kNoCameraIconOpacity;

        if (option.state == CameraHotspotDisplayOption::State::disabled)
            iconOpacity *= nx::style::Hints::kDisabledItemOpacity;

        QnScopedPainterOpacityRollback opacityRollback(painter, iconOpacity);

        const auto icon = option.decoration.value<QIcon>();
        QPixmap pixmap = icon.pixmap(nx::style::Metrics::kDefaultIconSize, QIcon::Selected);

        if (option.cameraState == CameraHotspotDisplayOption::CameraState::invalid)
            pixmap = qnSkin->colorize(pixmap, kInvalidColor);

        const auto iconOffset = -Geometry::toPoint(kHotspotItemIconSize) / 2.0 - QPoint(0, -1);
        painter->drawPixmap(origin + iconOffset, pixmap);
    }
}

} // namespace camera_hotspots
} // namespace nx::vms::client::desktop
