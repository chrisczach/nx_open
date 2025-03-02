// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

import QtQuick
import QtQuick.Controls

import Nx

import "settings.js" as Settings
import "components"

Item
{
    id: settingsView

    signal valuesChanged()
    signal valuesEdited(Item activeItem)

    property Item contentItem: null
    property bool contentEnabled: true
    property bool contentVisible: true
    property Item scrollBarParent: null
    property Item headerItem: null
    property Item placeholderItem: null
    property alias thumbnailSource: sharedData.thumbnailSource

    implicitWidth: 100
    implicitHeight: contentItem ? contentItem.heightHint : 0

    QtObject
    {
        id: impl
        property bool valuesChangedEnabled: true
        property var currentSectionScrollBar: null
    }

    QtObject
    {
        id: sharedData

        property var thumbnailSource: null
        property var resourceId: thumbnailSource ? thumbnailSource.cameraId : NxGlobals.uuid("")
    }

    function loadModel(model, initialValues, sectionPath, restoreScrollPosition)
    {
        const focusState = takeFocusState()
        const scrollPosition =
            impl.currentSectionScrollBar ? impl.currentSectionScrollBar.position : 0.0

        if (contentItem)
        {
            contentItem.visible = false
            contentItem.parent = null
            contentItem.focus = false //< Signals may happen.
            contentItem.destroy()
        }

        contentItem = Settings.createItems(settingsView, model)
        if (!contentItem)
            return

        contentItem.contentEnabled = Qt.binding(function() { return contentEnabled })
        contentItem.contentVisible = Qt.binding(function() { return contentVisible })
        contentItem.scrollBarParent = Qt.binding(function() { return scrollBarParent })
        contentItem.extraHeaderItem = Qt.binding(function() { return headerItem })
        contentItem.placeholderItem = Qt.binding(function() { return placeholderItem })

        if (sectionPath)
            settingsView.selectSection(sectionPath)

        if (restoreScrollPosition && impl.currentSectionScrollBar)
        {
            let scroll =
                () =>
                {
                    impl.currentSectionScrollBar.sizeChanged.disconnect(scroll)
                    impl.currentSectionScrollBar.position = scrollPosition
                }

            // Scroll after resize.
            impl.currentSectionScrollBar.sizeChanged.connect(scroll)
        }

        if (initialValues)
            setValues(initialValues)

        if (focusState)
            Settings.setFocusState(contentItem, focusState)
    }

    function getValues()
    {
        return contentItem && Settings.getValues(contentItem)
    }

    function setValues(values)
    {
        if (!contentItem)
            return

        impl.valuesChangedEnabled = false
        Settings.setValues(contentItem, values)
        valuesChanged()
        impl.valuesChangedEnabled = true
    }

    function _emitValuesChanged()
    {
        if (impl.valuesChangedEnabled)
            valuesChanged()
    }

    function triggerValuesEdited(activeItem)
    {
        if (!impl.valuesChangedEnabled || !settingsView.enabled)
            return

        valuesEdited(activeItem)
        valuesChanged()
    }

    function selectSection(sectionPath)
    {
        let stack = contentItem && contentItem.sectionStack
        if (!stack)
            return

        for (let i = 0; i < sectionPath.length; ++i)
        {
            var sectionIndex = sectionPath[i] + 1
            if (sectionIndex < stack.count)
            {
                stack.currentIndex = sectionIndex
                stack = stack.children[sectionIndex].sectionStack
            }
        }

        if (stack)
        {
            stack.currentIndex = 0
            impl.currentSectionScrollBar = stack.verticalScrollBar
        }
    }

    function setErrors(errors)
    {
        if (contentItem)
            Settings.setErrors(contentItem, errors)
    }

    function takeFocusState()
    {
        impl.valuesChangedEnabled = false
        let focusState = contentItem ? Settings.takeFocusState(contentItem) : null
        impl.valuesChangedEnabled = true
        return focusState
    }
}
