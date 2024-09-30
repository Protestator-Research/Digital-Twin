//
// Created by Moritz Herzog on 26.09.24.
//

#include "VariableViewModel.h"
#include "VariableViewModelItem.h"

namespace DigitalTwin::Client {

    VariableViewModel::VariableViewModel(QObject *parent) : QAbstractItemModel(parent) {
        Model = nullptr;
        RootItem = new VariableViewModelItem(QObject::tr("Components/Variables").toStdString());
    }

    VariableViewModel::~VariableViewModel() {
        delete RootItem;
    }

    QVariant VariableViewModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return {};

        VariableViewModelItem *item = const_cast<VariableViewModelItem *>(static_cast<const VariableViewModelItem *>(index.internalPointer()));
        return item->data();
    }

    Qt::ItemFlags VariableViewModel::flags(const QModelIndex &index) const {
        return index.isValid()
               ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
    }

    QVariant VariableViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
        return orientation == Qt::Horizontal && role == Qt::DisplayRole
               ? RootItem->data() : QVariant{};
    }

    QModelIndex VariableViewModel::index(int row, int column, const QModelIndex &parent) const {
        if (!hasIndex(row, column, parent))
            return {};

        VariableViewModelItem *parentItem = parent.isValid()
                                          ? static_cast<VariableViewModelItem*>(parent.internalPointer())
                                          : RootItem;

        if (auto *childItem = parentItem->child(row))
            return createIndex(row, column, childItem);

        return {};
    }

    QModelIndex VariableViewModel::parent(const QModelIndex &index) const {
        if (!index.isValid())
            return {};

        auto *childItem = static_cast<VariableViewModelItem*>(index.internalPointer());
        VariableViewModelItem *parentItem = childItem->parent();

        return parentItem != RootItem
               ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
    }

    int VariableViewModel::rowCount(const QModelIndex &parent) const {
        if (parent.column() > 0)
            return 0;

        auto *parentItem = const_cast<VariableViewModelItem *>(parent.isValid()
                                                                            ? static_cast<const VariableViewModelItem *>(parent.internalPointer())
                                                                            : RootItem);

        return parentItem->childCount();
    }

    int VariableViewModel::columnCount(const QModelIndex &parent) const {
        return 1;
    }

    void VariableViewModel::setDigitalTwinModel(DigitalTwin::Model::DigitalTwinModel *model) {
        Model = model;
        getUpdatesFromModel();
        Model->setUpdateModelFunction([this]{getUpdatesFromModel();});
    }

    void VariableViewModel::getUpdatesFromModel() {
        auto componentsVector = Model->getAllComponents();
        beginInsertRows(QModelIndex(), 0, componentsVector.size()-1);
        for(const auto component : componentsVector){
            RootItem->appendComponent(dynamic_cast<DigitalTwin::Model::Component*>(component));
        }
        endInsertRows();
    }

    VariableViewModelItem* VariableViewModel::getItemFromIndex(const QModelIndex &index) const {
        if((index.isValid())&&(index.internalPointer()!=RootItem)) {
            return (VariableViewModelItem*)(index.internalPointer());
        }
        return nullptr;
    }
} // DigitalTwin