//
// Created by Moritz Herzog on 26.09.24.
//

#ifndef DIGITALTWIN_VARIABLEVIEWMODEL_H
#define DIGITALTWIN_VARIABLEVIEWMODEL_H

#include <QAbstractItemModel>
#include <Model/DigitalTwinModel.h>

namespace DigitalTwin::Client{
    class VariableViewModelItem;
}

namespace DigitalTwin::Client {
    class VariableViewModel : public QAbstractItemModel {
        Q_OBJECT
    public:
        Q_DISABLE_COPY_MOVE(VariableViewModel)

        explicit VariableViewModel(QObject* parent = nullptr);
        virtual ~VariableViewModel();

        QVariant data(const QModelIndex &index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = {}) const override;
        int columnCount(const QModelIndex &parent = {}) const override;

        void setDigitalTwinModel(DigitalTwin::Model::DigitalTwinModel* model);
        VariableViewModelItem* getItemFromIndex(const QModelIndex &index) const;

    private:
        void getUpdatesFromModel();

        DigitalTwin::Model::DigitalTwinModel * Model;
        VariableViewModelItem* RootItem;
    };
}

#endif //DIGITALTWIN_VARIABLEVIEWMODEL_H
