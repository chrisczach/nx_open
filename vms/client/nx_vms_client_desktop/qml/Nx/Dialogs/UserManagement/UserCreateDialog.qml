// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import Nx 1.0
import Nx.Controls 1.0
import Nx.Models 1.0

import nx.client.desktop 1.0
import nx.vms.client.core 1.0
import nx.vms.client.desktop 1.0

import ".."
import "Tabs"

DialogWithState
{
    id: dialog
    objectName: "userCreateDialog" //< For autotesting.

    modality: Qt.ApplicationModal

    width: minimumWidth
    height: minimumHeight

    minimumWidth: 765
    minimumHeight: 672

    color: ColorTheme.colors.dark7

    // State properties.
    property alias userType: generalSettings.userType
    property bool isSelf: false //< Not used by current dialog type.
    property alias userId: membersModel.userId
    property bool deleteAvailable: false
    property bool auditAvailable: false
    property alias login: generalSettings.login
    property bool loginEditable: true
    property alias fullName: generalSettings.fullName
    property bool fullNameEditable: true
    property alias email: generalSettings.email
    property bool emailEditable: true
    property string password: generalSettings.password
    property bool passwordEditable: true
    property alias userEnabled: generalSettings.userEnabled
    property bool userEnabledEditable: true
    property alias allowInsecure: generalSettings.allowInsecure
    property bool allowInsecureEditable: true
    property alias parentGroups: membersModel.parentGroups
    property bool parentGroupsEditable: true
    property alias globalPermissions: globalPermissionsModel.globalPermissions
    property alias sharedResources: membersModel.sharedResources
    property bool editable: true  //< Not used by current dialog type.

    // Mapped to dialog property.
    property alias tabIndex: tabControl.currentTabIndex
    property var self

    signal groupClicked(var id)

    title: qsTr("New User")

    validateFunc: () =>
    {
        if (generalSettings.validate())
            return true

        dialog.tabIndex = 0
        return false
    }

    MembersModel
    {
        id: membersModel
    }

    DialogTabControl
    {
        id: tabControl

        anchors.fill: parent
        anchors.bottomMargin: buttonBox.height

        dialogLeftPadding: dialog.leftPadding
        dialogRightPadding: dialog.rightPadding

        Tab
        {
            button: DialogTabButton
            {
                text: qsTr("General")
            }

            page: UserCreateTab
            {
                id: generalSettings
                anchors.fill: parent
                self: dialog.self
            }
        }
        Tab
        {
            button: DialogTabButton
            {
                text: qsTr("Groups")
            }

            page: ParentGroupsTab
            {
                id: groupsSettings
                anchors.fill: parent

                model: membersModel

                onAddGroupClicked: dialog.groupClicked(null)
            }
        }
        Tab
        {
            button: DialogTabButton
            {
                text: qsTr("Resources")
            }

            page: PermissionsTab
            {
                id: permissionSettings
                anchors.fill: parent

                buttonBox: buttonBox
                editingContext: membersModel.editingContext
            }
        }

        Tab
        {
            button: DialogTabButton
            {
                text: qsTr("Global Permissions")
            }

            page: GlobalPermissionsTab
            {
                id: globalPermissionSettings
                anchors.fill: parent

                model: GlobalPermissionsModel
                {
                    id: globalPermissionsModel
                    context: membersModel.editingContext
                }
            }
        }
    }

    buttonBox: DialogButtonBox
    {
        id: buttonBox
        buttonLayout: DialogButtonBox.KdeLayout

        standardButtons: DialogButtonBox.Cancel

        Button
        {
            text: qsTr("Add User")
            isAccentButton: true
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }
}
