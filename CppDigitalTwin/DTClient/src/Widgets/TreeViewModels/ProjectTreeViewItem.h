//
// Created by Moritz Herzog on 15.05.24.
//

#ifndef DIGITALTWIN_PROJECTTREEVIEWITEM_H
#define DIGITALTWIN_PROJECTTREEVIEWITEM_H

#include <QVariant>

namespace SysMLv2::Entities{
    class Project;
    class DigitalTwin;
}

namespace DigitalTwin::Client::ViewModels {
    class ProjectTreeViewItem {
    public:
        ProjectTreeViewItem() = default;
        explicit ProjectTreeViewItem(SysMLv2::Entities::Project* project, ProjectTreeViewItem* parent = nullptr);
        explicit ProjectTreeViewItem(SysMLv2::Entities::DigitalTwin* digitalTwin, ProjectTreeViewItem* parent = nullptr);

        ~ProjectTreeViewItem();

        ProjectTreeViewItem* child(int row);
        int childCount();
        int columnCount();
        QVariant data();
        int row();
        ProjectTreeViewItem* parentItem();

        void appendProject(SysMLv2::Entities::Project* project);
        SysMLv2::Entities::Project* getProject() const;
        void appendDigitalTwin(SysMLv2::Entities::DigitalTwin* digitalTwin);
        SysMLv2::Entities::DigitalTwin* getDigitalTwin() const;

    private:
        std::vector<ProjectTreeViewItem*> ChildItems;
        SysMLv2::Entities::Project* ProjectData = nullptr;
        SysMLv2::Entities::DigitalTwin* DigitalTwinData = nullptr;
        ProjectTreeViewItem *Parent = nullptr;
    };
}


#endif //DIGITALTWIN_PROJECTTREEVIEWITEM_H
