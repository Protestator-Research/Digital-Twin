//
// Created by Moritz Herzog on 14.05.24.
//

#ifndef DIGITALTWIN_PROJECTTREEVIEWMODEL_H
#define DIGITALTWIN_PROJECTTREEVIEWMODEL_H

#include <QAbstractItemModel>
#include <vector>
#include "ProjectTreeViewItem.h"

namespace SysMLv2::Entities{
    class Project;
    class DigitalTwin;
}

namespace DigitalTwin::Client::ViewModels {
    class ProjectTreeViewModel : public QAbstractItemModel {
        Q_OBJECT
    public:
        Q_DISABLE_COPY_MOVE(ProjectTreeViewModel)

        ProjectTreeViewModel(QObject* parent = nullptr);
        virtual ~ProjectTreeViewModel();

        QVariant data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column,
                          const QModelIndex &parent = {}) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = {}) const override;
        int columnCount(const QModelIndex &parent = {}) const override;

        void setProjects(std::vector<SysMLv2::Entities::Project*> projects);
        void setDigitalTwinForProjectWithId(SysMLv2::Entities::Project* project, std::vector<SysMLv2::Entities::DigitalTwin*> twins);

    private:
        std::vector<SysMLv2::Entities::Project*> Projects;
        ProjectTreeViewItem* RootItem;
    };
}


#endif //DIGITALTWIN_PROJECTTREEVIEWMODEL_H
