//
// Created by Moritz Herzog on 14.05.24.
//

#include "ProjectTreeViewModel.h"

#include <sysmlv2/rest/entities/Project.h>
#include <QVariant>

namespace DigitalTwin::Client::ViewModels {
    ProjectTreeViewModel::ProjectTreeViewModel(QObject *parent) :
    QAbstractItemModel(parent) {
        RootItem = new ProjectTreeViewItem();
    }

    ProjectTreeViewModel::~ProjectTreeViewModel() {
        delete RootItem;
    }

    void ProjectTreeViewModel::setProjects(std::vector<std::shared_ptr<SysMLv2::Entities::Project>> projects) {
        if(projects.empty())
            return;

        beginInsertRows(QModelIndex(), 0, projects.size()-1);
        Projects = projects;

        for(auto project : Projects)
            RootItem->appendProject(project);

        endInsertRows();
        emit dataChanged(index(0,0), index(projects.size()-1,1), {Qt::DisplayRole});
    }

    QVariant ProjectTreeViewModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return {};

        ProjectTreeViewItem *item = const_cast<ProjectTreeViewItem *>(static_cast<const ProjectTreeViewItem *>(index.internalPointer()));
        return item->data();
    }

    Qt::ItemFlags ProjectTreeViewModel::flags(const QModelIndex &index) const {
        return index.isValid()
               ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
    }

    QVariant ProjectTreeViewModel::headerData(int, Qt::Orientation orientation, int role) const {
        return orientation == Qt::Horizontal && role == Qt::DisplayRole
               ? RootItem->data() : QVariant{};
    }

    QModelIndex ProjectTreeViewModel::index(int row, int column, const QModelIndex &parent) const {
        if (!hasIndex(row, column, parent))
            return {};

        ProjectTreeViewItem *parentItem = parent.isValid()
                               ? static_cast<ProjectTreeViewItem*>(parent.internalPointer())
                               : RootItem;

        if (auto *childItem = parentItem->child(row))
            return createIndex(row, column, childItem);

        return {};
    }

    QModelIndex ProjectTreeViewModel::parent(const QModelIndex &index) const {
        if (!index.isValid())
            return {};

        auto *childItem = static_cast<ProjectTreeViewItem*>(index.internalPointer());
        ProjectTreeViewItem *parentItem = childItem->parentItem();

        return parentItem != RootItem
               ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
    }

    int ProjectTreeViewModel::rowCount(const QModelIndex &parent) const {
        if (parent.column() > 0)
            return 0;

        ProjectTreeViewItem *parentItem = const_cast<ProjectTreeViewItem *>(parent.isValid()
                                                                            ? static_cast<const ProjectTreeViewItem *>(parent.internalPointer())
                                                                            : RootItem);

        return parentItem->childCount();
    }

    int ProjectTreeViewModel::columnCount(const QModelIndex &) const {
        return 1;
    }

    void ProjectTreeViewModel::setDigitalTwinForProjectWithId(SysMLv2::Entities::Project *project,
                                                              std::vector<SysMLv2::Entities::DigitalTwin *> twins) {
        beginInsertRows(QModelIndex(), 0, Projects.size()-1);

        for(int i = 0; i<RootItem->childCount(); i++){
            const auto item = RootItem->child(i);
            if((item->getProject()!= nullptr)&&(item->getProject()->getId()==project->getId())){
                for(auto twin : twins){
                    item->appendDigitalTwin(twin);
                }
            }
        }

        endInsertRows();

        emit dataChanged(index(0,0), index(Projects.size()-1,1), {Qt::DisplayRole});
    }

    ProjectTreeViewItem *ProjectTreeViewModel::getProjectTreeViewItemFromIndex(const QModelIndex &index) const {
        if((index.isValid())&&(index.internalPointer()!=RootItem)) {
            return (ProjectTreeViewItem*)(index.internalPointer());
        }
        return nullptr;
    }

    void ProjectTreeViewModel::clearAllElements() {
        const int lastElement  = Projects.size()-1;

        if (lastElement < 0)
            return;

        beginRemoveRows(QModelIndex(), 0, lastElement);
        RootItem->clearChildItems();
        Projects.clear();
        endRemoveRows();
        removeRows(0, lastElement, index(0,0));
    }
}
