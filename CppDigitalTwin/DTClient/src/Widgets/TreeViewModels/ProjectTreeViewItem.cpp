//
// Created by Moritz Herzog on 15.05.24.
//

#include "ProjectTreeViewItem.h"

#include <SysMLv2Standard/entities/Project.h>
#include <SysMLv2Standard/entities/DigitalTwin.h>
#include <algorithm>

namespace DigitalTwin::Client::ViewModels {
    ProjectTreeViewItem::ProjectTreeViewItem() :
    ProjectData(nullptr),
    DigitalTwinData(nullptr),
    Parent(nullptr)
    {     }

    ProjectTreeViewItem::ProjectTreeViewItem(SysMLv2::Entities::DigitalTwin *digitalTwin, ProjectTreeViewItem *parent) :
        ProjectData(nullptr),
        DigitalTwinData(digitalTwin),
        Parent(parent)
    {    }

    ProjectTreeViewItem::ProjectTreeViewItem(SysMLv2::Entities::Project *project, ProjectTreeViewItem *parent) :
        ProjectData(project),
        DigitalTwinData(nullptr),
        Parent(parent)
    {    }

    ProjectTreeViewItem *ProjectTreeViewItem::child(int row) {
        return row >= 0 && row < childCount() ? ChildItems.at(row) : nullptr;
    }

    int ProjectTreeViewItem::childCount() {
        return int(ChildItems.size());
    }

    int ProjectTreeViewItem::columnCount() {
        return 1;
    }

    QVariant ProjectTreeViewItem::data() {
        if(ProjectData != nullptr)
            return QVariant(QString::fromStdString(ProjectData->getName()));
        if(DigitalTwinData != nullptr)
            return QVariant(QString::fromStdString(DigitalTwinData->getName()));
        return QVariant("Projects");
    }

    int ProjectTreeViewItem::row() {
        if (Parent == nullptr)
            return 0;
        const auto it = std::find_if(Parent->ChildItems.cbegin(), Parent->ChildItems.cend(),
                                     [this](ProjectTreeViewItem *treeItem) {
                                         return treeItem == this;
                                     });

        if (it != Parent->ChildItems.cend())
            return std::distance(Parent->ChildItems.cbegin(), it);

        Q_ASSERT(false); // should not happen
        return -1;
    }

    ProjectTreeViewItem *ProjectTreeViewItem::parentItem() {
        return Parent;
    }

    void ProjectTreeViewItem::appendProject(SysMLv2::Entities::Project *project) {
        ChildItems.push_back(new ProjectTreeViewItem(project,this));
    }

    void ProjectTreeViewItem::appendDigitalTwin(SysMLv2::Entities::DigitalTwin *digitalTwin) {
        ChildItems.push_back(new ProjectTreeViewItem(digitalTwin, this));
    }

    ProjectTreeViewItem::~ProjectTreeViewItem() {
        for(auto item : ChildItems)
            delete item;

        ChildItems.clear();
    }

    SysMLv2::Entities::DigitalTwin *ProjectTreeViewItem::getDigitalTwin() const {
        return DigitalTwinData;
    }

    SysMLv2::Entities::Project *ProjectTreeViewItem::getProject() const {
        return ProjectData;
    }

    void ProjectTreeViewItem::clearChildItems() {
        for(auto item : ChildItems)
            delete item;

        ChildItems.clear();
    }
}
