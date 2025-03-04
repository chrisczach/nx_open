// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <QtCore/QHash>

#include <nx/vms/client/desktop/resource_properties/user/utils/access_subject_editing_context.h>
#include <nx/vms/common/system_context.h>


namespace nx::vms::client::desktop {

/**
 * Lazily generated sorted lists of members for each group. Lists are sorted according to UI spec.
 * This class emits signals when the cache is modified, but itself is not subscribed to any signals.
 */
class NX_VMS_CLIENT_DESKTOP_API MembersCache: public QObject
{
    Q_OBJECT

public:
    // Sorted lists of members. It's important that these containers support implicit sharing.
    struct Members
    {
        QList<QnUuid> users;
        QList<QnUuid> groups;
    };

    // Cached information for each member, which is used for sorting members.
    struct Info
    {
        QString name;
        QString description;
        bool isGroup = false;
        bool isLdap = false;
    };

public:
    MembersCache();
    virtual ~MembersCache();

    Info info(const QnUuid& id) const;
    Members sorted(const QnUuid& groupId = {}) const;
    void loadInfo(nx::vms::common::SystemContext* systemContext);

    static bool isPredefined(const QnUuid& groupId);

    void setContext(AccessSubjectEditingContext* context) { m_subjectContext = context; }

    void sortSubjects(QList<QnUuid>& subjects) const;

    int indexIn(const QList<QnUuid>& list, const QnUuid& id) const;

public slots:
    void modify(
        const QSet<QnUuid>& added,
        const QSet<QnUuid>& removed,
        const QSet<QnUuid>& groupsWithChangedMembers,
        const QSet<QnUuid>& subjectsWithChangedParents);

signals:
    void beginInsert(int at, const QnUuid& id, const QnUuid& parentId);
    void endInsert(int at, const QnUuid& id, const QnUuid& parentId);

    void beginRemove(int at, const QnUuid& id, const QnUuid& parentId);
    void endRemove(int at, const QnUuid& id, const QnUuid& parentId);

    void reset();

private:
    static MembersCache::Info infoFromContext(
        nx::vms::common::SystemContext* systemContext,
        const QnUuid& id);

    // Get sorted lists of users and groups from a set of subjects.
    Members sortedSubjects(const QSet<QnUuid>& subjectSet) const;

    std::function<bool(const QnUuid&, const QnUuid&)> lessFunc() const;

private:
    mutable QHash<QnUuid, Members> m_sortedCache;
    QHash<QnUuid, Info> m_info;
    AccessSubjectEditingContext* m_subjectContext = nullptr;
    nx::vms::common::SystemContext* m_systemContext = nullptr;
};

} // namespace nx::vms::client::desktop
