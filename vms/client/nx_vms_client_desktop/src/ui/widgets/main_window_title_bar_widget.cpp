// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "main_window_title_bar_widget.h"

#include <QtCore/QScopedValueRollback>

#include <QtGui/QDragMoveEvent>

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMenu>

#include <common/common_module.h>
#include <client_core/client_core_module.h>

#include <core/resource_access/resource_access_filter.h>
#include <core/resource_management/resource_pool.h>
#include <core/resource_management/layout_tour_manager.h>
#include <core/resource/media_resource.h>
#include <core/resource/media_server_resource.h>
#include <core/resource/layout_resource.h>
#include <utils/math/math.h>

#include <nx/vms/client/desktop/ui/actions/actions.h>
#include <ui/common/palette.h>
#include <ui/help/help_topic_accessor.h>
#include <nx/vms/client/desktop/style/helper.h>
#include <ui/widgets/cloud_status_panel.h>
#include <ui/widgets/layout_tab_bar.h>
#include <nx/vms/client/desktop/common/widgets/tool_button.h>
#include <nx/vms/client/desktop/ui/actions/action_manager.h>
#include <nx/vms/client/desktop/ui/common/color_theme.h>
#include <ui/workbench/workbench_layout.h>
#include <ui/workaround/hidpi_workarounds.h>
#include <ui/workbench/workbench_display.h>

#include <utils/common/delayed.h>
#include <utils/common/event_processors.h>
#include <nx/vms/client/desktop/utils/mime_data.h>
#include <nx/utils/app_info.h>

using namespace nx::vms::client::core;
using namespace nx::vms::client::desktop;
using namespace nx::vms::client::desktop::ui;

namespace {

const int kTitleBarHeight = 24;
const QSize kControlButtonSize(36, kTitleBarHeight);
const auto kTabBarButtonSize = QSize(kTitleBarHeight, kTitleBarHeight);

QFrame* newVLine()
{
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedWidth(1);
    return line;
}

void executeButtonMenu(ToolButton* invoker, QMenu* menu, const QPoint& offset = QPoint(0, 0))
{
    if (!menu || !invoker)
        return;

    menu->setProperty(nx::style::Properties::kMenuNoMouseReplayArea,
        QVariant::fromValue(QPointer<QWidget>(invoker)));

    QnHiDpiWorkarounds::showMenu(menu,
        QnHiDpiWorkarounds::safeMapToGlobal(invoker,
            invoker->rect().bottomLeft() + offset));

    invoker->setDown(false);
}

/* If a widget is visible directly or within a graphics proxy: */
bool isWidgetVisible(const QWidget* widget)
{
    if (!widget->isVisible())
        return false; //< widget is not visible to the screen or a proxy

    if (const auto proxy = widget->window()->graphicsProxyWidget())
    {
        if (!proxy->isVisible() || !proxy->scene())
            return false; //< proxy is not visible or doesn't belong to a scene

        for (const auto view: proxy->scene()->views())
        {
            if (isWidgetVisible(view))
                return true; //< scene is visible
        }

        return false; // scene has no visible views
    }

    return true; //< widget is visible directly to the screen
}

} // namespace

class QnMainWindowTitleBarWidgetPrivate: public QObject
{
    QnMainWindowTitleBarWidget* q_ptr;
    Q_DECLARE_PUBLIC(QnMainWindowTitleBarWidget)

public:
    QnMainWindowTitleBarWidgetPrivate(QnMainWindowTitleBarWidget* parent);

    void setSkipDoubleClick();

public:
    ToolButton* mainMenuButton = nullptr;
    QnLayoutTabBar* tabBar = nullptr;
    ToolButton* newTabButton = nullptr;
    ToolButton* currentLayoutsButton = nullptr;
    QnCloudStatusPanel* cloudPanel = nullptr;
    std::unique_ptr<MimeData> mimeData;
    bool skipDoubleClickFlag = false;

    /** There are some rare scenarios with looping menus. */
    bool isMenuOpened = false;

    QSharedPointer<QMenu> mainMenuHolder;
};

QnMainWindowTitleBarWidgetPrivate::QnMainWindowTitleBarWidgetPrivate(
    QnMainWindowTitleBarWidget* parent)
    :
    QObject(parent),
    q_ptr(parent)
{
}

void QnMainWindowTitleBarWidgetPrivate::setSkipDoubleClick()
{
    skipDoubleClickFlag = true;
}

QnMainWindowTitleBarWidget::QnMainWindowTitleBarWidget(
    QnWorkbenchContext* context,
    QWidget* parent)
    :
    base_type(parent),
    QnWorkbenchContextAware(context),
    d_ptr(new QnMainWindowTitleBarWidgetPrivate(this))
{
    Q_D(QnMainWindowTitleBarWidget);

    setFocusPolicy(Qt::NoFocus);
    setAutoFillBackground(true);
    setFixedHeight(kTitleBarHeight);
    setAcceptDrops(true);
    setPaletteColor(this, QPalette::Base, colorTheme()->color("dark7"));
    setPaletteColor(this, QPalette::Window, colorTheme()->color("dark7"));

    d->mainMenuButton = newActionButton(
        action::MainMenuAction,
        Qn::MainWindow_TitleBar_MainMenu_Help);
    connect(d->mainMenuButton, &ToolButton::justPressed, this,
        [this]()
        {
            menu()->trigger(action::MainMenuAction);
        });

    connect(action(action::MainMenuAction), &QAction::triggered, this,
        [this]()
        {
            Q_D(QnMainWindowTitleBarWidget);
            if (!isWidgetVisible(d->mainMenuButton))
                return;

            if (d->isMenuOpened)
                return;
            QScopedValueRollback<bool> guard(d->isMenuOpened, true);

            static const QPoint kVerticalOffset(0, 2);
            d->mainMenuHolder.reset(menu()->newMenu(action::MainScope, mainWindowWidget()));
            d->mainMenuButton->setDown(true);
            executeButtonMenu(d->mainMenuButton, d->mainMenuHolder.data(), kVerticalOffset);
        },
        Qt::QueuedConnection); //< QueuedConnection is needed here because 2 title bars
                               // (windowed/welcomescreen and fullscreen) are subscribed to
                               // MainMenuAction, and main menu must not change
                               // windowed/welcomescreen/fullscreen state BETWEEN their slots 
                               // processing MainMenuAction.
                               //
                               // TODO: #vkutin #gdm #ynikitenkov Lift this limitation in the
                               // future

    d->tabBar = new QnLayoutTabBar(this);
    d->tabBar->setFocusPolicy(Qt::NoFocus);
    d->tabBar->setFixedHeight(kTitleBarHeight);
    connect(d->tabBar, &QnLayoutTabBar::tabCloseRequested, d,
        &QnMainWindowTitleBarWidgetPrivate::setSkipDoubleClick);
    connect(d->tabBar, &QnLayoutTabBar::closeRequested, this,
        [this](QnWorkbenchLayout* layout)
        {
            menu()->trigger(action::CloseLayoutAction,
                QnWorkbenchLayoutList() << layout);
        });

    d->cloudPanel = new QnCloudStatusPanel(this);
    d->cloudPanel->setFocusPolicy(Qt::NoFocus);
    d->cloudPanel->setFixedHeight(kTitleBarHeight);

    /* Layout for window buttons that can be removed from the title bar. */
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(d->mainMenuButton);
    layout->addWidget(newVLine());
    layout->addWidget(d->tabBar);
    layout->addWidget(newVLine());

    d->newTabButton = newActionButton(
        action::OpenNewTabAction,
        Qn::MainWindow_TitleBar_NewLayout_Help,
        kTabBarButtonSize);

    d->currentLayoutsButton = newActionButton(
        action::OpenCurrentUserLayoutMenu,
        kTabBarButtonSize);
    connect(d->currentLayoutsButton, &ToolButton::justPressed, this,
        [this]()
        {
            Q_D(QnMainWindowTitleBarWidget);

            if (d->isMenuOpened)
                return;
            QScopedValueRollback<bool> guard(d->isMenuOpened, true);

            QScopedPointer<QMenu> layoutsMenu(menu()->newMenu(
                action::OpenCurrentUserLayoutMenu,
                action::TitleBarScope,
                mainWindowWidget()));

            executeButtonMenu(d->currentLayoutsButton, layoutsMenu.data());
        });

    layout->addWidget(d->newTabButton);
    layout->addWidget(d->currentLayoutsButton);
    layout->addStretch(1);
    layout->addSpacing(80);
    layout->addWidget(newVLine());
    layout->addWidget(d->cloudPanel);
    layout->addWidget(newVLine());
#ifdef ENABLE_LOGIN_TO_ANOTHER_SYSTEM_BUTTON
    layout->addWidget(newActionButton(
        action::OpenLoginDialogAction,
        Qn::Login_Help,
        kControlButtonSize));
#else
    layout->addSpacing(8);
#endif
    layout->addWidget(newActionButton(
        action::WhatsThisAction,
        Qn::MainWindow_ContextHelp_Help,
        kControlButtonSize));
    layout->addWidget(newActionButton(
        action::MinimizeAction,
        kControlButtonSize));
    layout->addWidget(newActionButton(
        action::EffectiveMaximizeAction,
        Qn::MainWindow_Fullscreen_Help,
        kControlButtonSize));
    layout->addWidget(newActionButton(
        action::ExitAction,
        kControlButtonSize));

    installEventHandler({this}, {QEvent::Resize, QEvent::Move},
        this, &QnMainWindowTitleBarWidget::geometryChanged);
}

QnMainWindowTitleBarWidget::~QnMainWindowTitleBarWidget()
{
}

int QnMainWindowTitleBarWidget::y() const
{
    return pos().y();
}

void QnMainWindowTitleBarWidget::setY(int y)
{
    QPoint pos_ = pos();

    if (pos_.y() != y)
    {
        move(pos_.x(), y);
        emit yChanged(y);
    }
}

QnLayoutTabBar* QnMainWindowTitleBarWidget::tabBar() const
{
    Q_D(const QnMainWindowTitleBarWidget);
    return d->tabBar;
}

void QnMainWindowTitleBarWidget::setTabBarStuffVisible(bool visible)
{
    Q_D(const QnMainWindowTitleBarWidget);
    d->tabBar->setVisible(visible);
    d->newTabButton->setVisible(visible);
    d->currentLayoutsButton->setVisible(visible);
    action(action::OpenNewTabAction)->setEnabled(visible);
}

void QnMainWindowTitleBarWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_D(QnMainWindowTitleBarWidget);

    base_type::mouseDoubleClickEvent(event);

#ifndef Q_OS_MACX
    if (d->skipDoubleClickFlag)
    {
        d->skipDoubleClickFlag = false;
        return;
    }

    menu()->trigger(action::EffectiveMaximizeAction);
    event->accept();
#endif
}

void QnMainWindowTitleBarWidget::dragEnterEvent(QDragEnterEvent* event)
{
    Q_D(QnMainWindowTitleBarWidget);
    d->mimeData.reset(new MimeData{event->mimeData(), resourcePool()});
    if (d->mimeData->isEmpty())
        return;

    event->acceptProposedAction();
}

void QnMainWindowTitleBarWidget::dragMoveEvent(QDragMoveEvent* event)
{
    Q_D(QnMainWindowTitleBarWidget);
    if (d->mimeData->isEmpty())
        return;

    if (!qBetween(d->tabBar->pos().x(), event->pos().x(), d->cloudPanel->pos().x()))
        return;

    event->acceptProposedAction();
}

void QnMainWindowTitleBarWidget::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    Q_D(QnMainWindowTitleBarWidget);
    d->mimeData.reset();
}

void QnMainWindowTitleBarWidget::dropEvent(QDropEvent* event)
{
    Q_D(QnMainWindowTitleBarWidget);
    if (d->mimeData->isEmpty())
        return;

    const auto layoutTours = layoutTourManager()->tours(d->mimeData->entities());
    for (const auto& tour: layoutTours)
        menu()->trigger(action::ReviewLayoutTourAction, {UuidRole, tour.id});

    resourcePool()->addNewResources(d->mimeData->resources());

    menu()->triggerIfPossible(action::OpenInNewTabAction, d->mimeData->resources());

    d->mimeData.reset();
    event->acceptProposedAction();
}

ToolButton* QnMainWindowTitleBarWidget::newActionButton(
    action::IDType actionId,
    int helpTopicId,
    const QSize& fixedSize)
{
    auto button = new ToolButton(this);

    button->setDefaultAction(action(actionId));
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustIconSize();
    button->setFixedSize(fixedSize.isEmpty() ? button->iconSize() : fixedSize);
    button->setAutoRaise(true);

    if (helpTopicId != Qn::Empty_Help)
        setHelpTopic(button, helpTopicId);

    return button;
}

ToolButton* QnMainWindowTitleBarWidget::newActionButton(
    action::IDType actionId,
    const QSize& fixedSize)
{
    return newActionButton(actionId, Qn::Empty_Help, fixedSize);
}
