// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "user_settings_global.h"

#include <QtQml/QQmlEngine>

#include <core/resource/user_resource.h>
#include <core/resource_management/resource_pool.h>
#include <nx/vms/client/core/common/utils/cloud_url_helper.h>
#include <nx/vms/client/desktop/common/utils/password_information.h>
#include <nx/vms/client/desktop/common/utils/validators.h>
#include <nx/vms/client/desktop/ui/common/color_theme.h>

namespace nx::vms::client::desktop {

namespace {

QObject* createUserSettingsGlobal(QQmlEngine*, QJSEngine*)
{
    return new UserSettingsGlobal();
}

} // namespace

const QString UserSettingsGlobal::kUsersSection = "U";
const QString UserSettingsGlobal::kGroupsSection = "G";
const QString UserSettingsGlobal::kBuiltInGroupsSection = "B";
const QString UserSettingsGlobal::kCustomGroupsSection = "C";

UserSettingsGlobal::UserSettingsGlobal()
{
}

UserSettingsGlobal::~UserSettingsGlobal()
{
}

void UserSettingsGlobal::registerQmlTypes()
{
    qmlRegisterSingletonType<UserSettingsGlobal>(
        "nx.vms.client.desktop",
        1,
        0,
        "UserSettingsGlobal",
        &createUserSettingsGlobal);

    qRegisterMetaType<PasswordStrengthData>();
}

Q_INVOKABLE QUrl UserSettingsGlobal::accountManagementUrl() const
{
    core::CloudUrlHelper urlHelper(
        nx::vms::utils::SystemUri::ReferralSource::DesktopClient,
        nx::vms::utils::SystemUri::ReferralContext::SettingsDialog);

    return urlHelper.accountManagementUrl();
}

QString UserSettingsGlobal::validatePassword(const QString& password)
{
    auto validateFunction = defaultPasswordValidator(/*allowEmpty*/ false);
    const auto result = validateFunction(password);
    return result.state != QValidator::Acceptable ? result.errorMessage : "";
}

PasswordStrengthData UserSettingsGlobal::passwordStrength(const QString& password)
{
    PasswordInformation info(password, nx::utils::passwordStrength);
    PasswordStrengthData result;

    result.text = info.text().toUpper();
    result.hint = info.hint();

    switch (info.acceptance())
    {
        case nx::utils::PasswordAcceptance::Good:
            result.background = colorTheme()->color("green_core");
            result.accepted = true;
            break;

        case nx::utils::PasswordAcceptance::Acceptable:
            result.background = colorTheme()->color("yellow_core");
            result.accepted = true;
            break;

        default:
            result.background = colorTheme()->color("red_core");
            result.accepted = false;
            break;
    }

    return result;
}

UserSettingsGlobal::UserType UserSettingsGlobal::getUserType(const QnUserResourcePtr& user)
{
    if (user->isCloud())
        return UserSettingsGlobal::CloudUser;

    if (user->isLdap())
        return UserSettingsGlobal::LdapUser;

    return UserSettingsGlobal::LocalUser;
}

} // namespace nx::vms::client::desktop
