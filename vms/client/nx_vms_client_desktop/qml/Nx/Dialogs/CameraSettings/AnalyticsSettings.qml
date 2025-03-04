// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

import QtQuick 2.11
import QtQuick.Layouts 1.11

import Nx 1.0
import Nx.Controls 1.0
import Nx.Controls.NavigationMenu 1.0
import Nx.Core 1.0
import Nx.InteractiveSettings 1.0
import Nx.Items 1.0
import Nx.Utils 1.0

import nx.vms.client.core 1.0
import nx.vms.client.desktop 1.0

Item
{
    id: analyticsSettings

    property var store: null
    property var backend: null
    property bool loading: false
    property bool supportsDualStreaming: false
    readonly property bool isDeviceDependent: viewModel.currentEngineInfo !== undefined
        && viewModel.currentEngineInfo.isDeviceDependent

    readonly property var currentEngineId: viewModel.currentEngineId
    property var resourceId: NxGlobals.uuid("")

    Connections
    {
        target: store

        function onStateModified()
        {
            const resourceId = store.resourceId()
            if (resourceId.isNull())
                return

            loading = store.analyticsSettingsLoading()
            if (loading)
                return

            supportsDualStreaming = store.dualStreamingEnabled()
            analyticsSettings.resourceId = resourceId

            const currentEngineId = store.currentAnalyticsEngineId()
            viewModel.enabledEngines = store.userEnabledAnalyticsEngines()
            viewModel.updateState(
                store.analyticsEngines(),
                currentEngineId,
                store.deviceAgentSettingsModel(currentEngineId),
                store.deviceAgentSettingsValues(currentEngineId),
                store.deviceAgentSettingsErrors(currentEngineId))

            if (currentEngineId)
            {
                analyticsSettingsView.settingsViewHeader.currentStreamIndex =
                    store.analyticsStreamIndex(currentEngineId)
            }

            banner.visible = !store.recordingEnabled() && viewModel.enabledEngines.length !== 0
        }
    }

    MediaResourceHelper
    {
        id: mediaResourceHelper

        resourceId: analyticsSettings.resourceId
    }

    AnalyticsSettingsViewModel
    {
        id: viewModel

        onEngineRequested: (engineId) =>
        {
            if (store)
                store.setCurrentAnalyticsEngineId(engineId ?? NxGlobals.uuid(""))
        }
    }

    AnalyticsSettingsMenu
    {
        id: navigationMenu

        width: 240
        height: parent.height - banner.height

        viewModel: viewModel
    }

    AnalyticsSettingsView
    {
        id: analyticsSettingsView

        viewModel: viewModel

        x: navigationMenu.width + 16
        y: 16
        width: parent.width - x - 24
        height: parent.height - 16 - banner.height

        settingsView
        {
            enabled: !loading
            contentEnabled: settingsViewHeader.checked || isDeviceDependent
            contentVisible: settingsView.contentEnabled
            scrollBarParent: scrollBarsParent

            thumbnailSource: RoiCameraThumbnail
            {
                cameraId: analyticsSettings.resourceId
                active: visible
            }

            onValuesEdited: (activeItem) =>
            {
                const parameters = activeItem && activeItem.parametersModel
                    ? backend.requestParameters(activeItem.parametersModel)
                    : {}

                if (parameters == null)
                    return

                store.setDeviceAgentSettingsValues(
                    currentEngineId,
                    activeItem ? activeItem.name : "",
                    analyticsSettingsView.settingsView.getValues(),
                    parameters)
            }
        }

        settingsViewHeader
        {
            checkable: !isDeviceDependent && !!currentEngineId
            refreshable: settingsViewHeader.checked || !settingsViewHeader.checkable
            refreshing: analyticsSettings.loading
            streamSelectorVisible:
                supportsDualStreaming && settingsViewHeader.currentStreamIndex >= 0
            streamSelectorEnabled: settingsView.contentEnabled

            onEnableSwitchClicked:
            {
                if (currentEngineId)
                    setEngineEnabled(currentEngineId, !settingsViewHeader.checked)
            }

            onRefreshButtonClicked:
            {
                if (currentEngineId)
                    store.refreshDeviceAgentSettings(currentEngineId)
            }

            onCurrentStreamIndexChanged:
            {
                if (currentEngineId)
                {
                    store.setAnalyticsStreamIndex(
                        currentEngineId,
                        settingsViewHeader.currentStreamIndex)
                }
            }
        }

        settingsViewPlaceholder
        {
            header: qsTr("This plugin has no settings for this camera.")
            description: qsTr("Check System Administration settings to configure this plugin.")
            loading: analyticsSettings.loading
        }
    }

    Item
    {
        id: scrollBarsParent
        width: parent.width
        height: banner.y
    }

    Banner
    {
        id: banner

        height: visible ? implicitHeight : 0
        visible: false
        anchors.bottom: parent.bottom
        text: qsTr("Camera analytics will work only when camera is being viewed."
            + " Enable recording to make it work all the time.")
    }

    function setEngineEnabled(engineId, enabled)
    {
        var engines = viewModel.enabledEngines.slice(0)
        if (enabled)
        {
            engines.push(engineId)
        }
        else
        {
            const index = engines.indexOf(engineId)
            if (index !== -1)
                engines.splice(index, 1)
        }
        store.setUserEnabledAnalyticsEngines(engines)
    }
}
