// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "client_core_module.h"

#include <QtCore/QStandardPaths>
#include <QtQml/QQmlEngine>

#include <api/common_message_processor.h>
#include <common/common_module.h>
#include <core/dataprovider/data_provider_factory.h>
#include <nx/core/access/access_types.h>
#include <nx/utils/app_info.h>
#include <nx/utils/log/log.h>
#include <nx/utils/timer_manager.h>
#include <nx/vms/client/core/application_context.h>
#include <nx/vms/client/core/ini.h>
#include <nx/vms/client/core/network/local_network_interfaces_manager.h>
#include <nx/vms/client/core/network/network_module.h>
#include <nx/vms/client/core/network/remote_connection.h>
#include <nx/vms/client/core/network/session_token_terminator.h>
#include <nx/vms/client/core/resource/screen_recording/audio_only/desktop_audio_only_resource.h>
#include <nx/vms/client/core/settings/client_core_settings.h>
#include <nx/vms/client/core/system_context.h>
#include <nx/vms/client/core/utils/operation_manager.h>
#include <nx/vms/client/core/watchers/known_server_connections.h>

using namespace nx::vms::client::core;

static QnClientCoreModule* s_instance = nullptr;

struct QnClientCoreModule::Private
{
    std::unique_ptr<QnCommonModule> commonModule;
    std::unique_ptr<NetworkModule> networkModule;
    std::unique_ptr<QnDataProviderFactory> resourceDataProviderFactory;
    std::unique_ptr<SessionTokenTerminator> sessionTokenTerminator;
};

//-------------------------------------------------------------------------------------------------
// QnClientCoreModule

QnClientCoreModule::QnClientCoreModule(
    SystemContext* systemContext)
    :
    base_type(),
    d(new Private())
{
    if (s_instance)
        NX_ERROR(this, "Singleton is created more than once.");
    else
        s_instance = this;

    d->commonModule = std::make_unique<QnCommonModule>(systemContext);

    d->commonModule->store(new nx::vms::client::core::LocalNetworkInterfacesManager());
    d->commonModule->store(new watchers::KnownServerConnections(d->commonModule.get()));
    d->commonModule->store(new OperationManager());

    systemContext->storeToQmlContext(appContext()->qmlEngine()->rootContext());

    d->resourceDataProviderFactory.reset(new QnDataProviderFactory());


    d->resourceDataProviderFactory->registerResourceType<DesktopAudioOnlyResource>();

    d->sessionTokenTerminator = std::make_unique<SessionTokenTerminator>();
}

QnClientCoreModule::~QnClientCoreModule()
{
    if (s_instance == this)
        s_instance = nullptr;
}

QnClientCoreModule* QnClientCoreModule::instance()
{
    return s_instance;
}

void QnClientCoreModule::initializeNetworking(
    nx::vms::api::PeerType peerType,
    Qn::SerializationFormat serializationFormat)
{
    d->networkModule = std::make_unique<NetworkModule>(
        d->commonModule.get(),
        peerType,
        serializationFormat);

    connect(&settings()->certificateValidationLevel, &Settings::BaseProperty::changed, this,
        [this](nx::utils::property_storage::BaseProperty* /*property*/)
        {
            d->networkModule->reinitializeCertificateStorage();
        });
}

NetworkModule* QnClientCoreModule::networkModule() const
{
    return d->networkModule.get();
}

QnResourcePool* QnClientCoreModule::resourcePool() const
{
    return d->commonModule->resourcePool();
}

QnCommonModule* QnClientCoreModule::commonModule() const
{
    return d->commonModule.get();
}

QnDataProviderFactory* QnClientCoreModule::dataProviderFactory() const
{
    return d->resourceDataProviderFactory.get();
}

QQmlEngine* QnClientCoreModule::mainQmlEngine() const
{
    return appContext()->qmlEngine();
}

SessionTokenTerminator* QnClientCoreModule::sessionTokenTerminator() const
{
    return d->sessionTokenTerminator.get();
}
