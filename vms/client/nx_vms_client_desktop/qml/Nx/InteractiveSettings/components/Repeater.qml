// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

import QtQuick 2.0

import Nx.Controls 1.0

import "private"
import "private/utils.js" as Utils
import "../settings.js" as Settings

Item
{
    id: control
    property string addButtonCaption: ""
    property string deleteButtonCaption: ""

    readonly property Item childrenItem: column

    readonly property real buttonSpacing: 24

    property bool fillWidth: true

    implicitHeight: column.implicitHeight + addButton.implicitHeight + buttonSpacing

    property int visibleItemsCount: 0

    property bool filled: false

    function processFilledChanged()
    {
        const lastFilled = lastFilledItemIndex()
        visibleItemsCount = Math.max(1, visibleItemsCount, lastFilled + 1)
        filled = (lastFilled >= 0)
    }

    LabeledColumnLayout
    {
        id: column
        width: parent.width
        layoutControl: control

        onLayoutItemsChanged: initialVisibilityUpdateTimer.restart()
    }

    Row
    {
        x: column.labelWidth + 8
        y: column.y + column.height + buttonSpacing
        spacing: 12

        Button
        {
            id: addButton

            text: addButtonCaption || qsTr("Add")
            visible: visibleItemsCount < column.layoutItems.length
            iconUrl: "qrc:///skin/buttons/plus.png"

            onClicked:
            {
                if (visibleItemsCount < column.layoutItems.length)
                    ++visibleItemsCount
            }
        }

        Button
        {
            id: removeButton

            text: deleteButtonCaption || qsTr("Delete")
            visible: visibleItemsCount > 1
            iconUrl: "qrc:///skin/buttons/minus.png"

            onClicked:
            {
                if (!visibleItemsCount)
                    return

                --visibleItemsCount
                let itemToReset = column.layoutItems[visibleItemsCount]
                Settings.resetValues(itemToReset)
            }
        }
    }

    Timer
    {
        // Queued update of visible items count.
        id: initialVisibilityUpdateTimer
        interval: 0
        running: false
        onTriggered:
        {
            visibleItemsCount = Math.max(1, lastFilledItemIndex() + 1)
            updateVisibility()
        }
    }

    onVisibleItemsCountChanged: updateVisibility()

    function lastFilledItemIndex()
    {
        for (var i = column.layoutItems.length - 1; i >= 0; --i)
        {
            if (Utils.isItemFilled(column.layoutItems[i]))
                return i
        }
        return -1
    }

    function updateVisibility()
    {
        for (var i = 0; i < column.layoutItems.length; ++i)
            column.layoutItems[i].opacity = (i < visibleItemsCount)
    }
}
