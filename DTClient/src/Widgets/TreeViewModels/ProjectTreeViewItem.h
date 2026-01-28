//
// Created by Moritz Herzog on 15.05.24.
//

#ifndef DIGITALTWIN_PROJECTTREEVIEWITEM_H
#define DIGITALTWIN_PROJECTTREEVIEWITEM_H

#include <QVariant>

namespace SysMLv2::REST{
    class Project;
    class DigitalTwin;
}

namespace DigitalTwin::Client::ViewModels {
    class ProjectTreeViewItem {
    public:
        ProjectTreeViewItem();
        explicit ProjectTreeViewItem(std::shared_ptr<SysMLv2::REST::Project> project, ProjectTreeViewItem* parent = nullptr);
        explicit ProjectTreeViewItem(SysMLv2::REST::DigitalTwin* digitalTwin, ProjectTreeViewItem* parent = nullptr);

        ~ProjectTreeViewItem();

        ProjectTreeViewItem* child(int row);
        int childCount();
        int columnCount();
        QVariant data();
        int row();
        ProjectTreeViewItem* parentItem();

        void appendProject(std::shared_ptr<SysMLv2::REST::Project> project);
        std::shared_ptr<SysMLv2::REST::Project> getProject() const;
        void appendDigitalTwin(SysMLv2::REST::DigitalTwin* digitalTwin);
        SysMLv2::REST::DigitalTwin* getDigitalTwin() const;

        void clearChildItems();

    private:
        std::vector<ProjectTreeViewItem*> ChildItems;
        std::shared_ptr<SysMLv2::REST::Project> ProjectData;
        SysMLv2::REST::DigitalTwin* DigitalTwinData;
        ProjectTreeViewItem *Parent;
    };
}


#endif //DIGITALTWIN_PROJECTTREEVIEWITEM_H
