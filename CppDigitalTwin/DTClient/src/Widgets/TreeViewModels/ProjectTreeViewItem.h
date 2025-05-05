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
        ProjectTreeViewItem();
        explicit ProjectTreeViewItem(std::shared_ptr<SysMLv2::Entities::Project> project, ProjectTreeViewItem* parent = nullptr);
        explicit ProjectTreeViewItem(SysMLv2::Entities::DigitalTwin* digitalTwin, ProjectTreeViewItem* parent = nullptr);

        ~ProjectTreeViewItem();

        ProjectTreeViewItem* child(int row);
        int childCount();
        int columnCount();
        QVariant data();
        int row();
        ProjectTreeViewItem* parentItem();

        void appendProject(std::shared_ptr<SysMLv2::Entities::Project> project);
        std::shared_ptr<SysMLv2::Entities::Project> getProject() const;
        void appendDigitalTwin(SysMLv2::Entities::DigitalTwin* digitalTwin);
        SysMLv2::Entities::DigitalTwin* getDigitalTwin() const;

        void clearChildItems();

    private:
        std::vector<ProjectTreeViewItem*> ChildItems;
        std::shared_ptr<SysMLv2::Entities::Project> ProjectData;
        SysMLv2::Entities::DigitalTwin* DigitalTwinData;
        ProjectTreeViewItem *Parent;
    };
}


#endif //DIGITALTWIN_PROJECTTREEVIEWITEM_H
