//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_TABCONTENTWIDGET_H
#define DIGITALTWIN_TABCONTENTWIDGET_H

#include <QWidget>
#include <QChartView>
#include <QLineSeries>

namespace DigitalTwin::Client {
    QT_BEGIN_NAMESPACE
    namespace Ui { class TabContentWidget; }
    QT_END_NAMESPACE

    class TabContentWidget : public QWidget {
    Q_OBJECT

    public:
        explicit TabContentWidget(QWidget *parent = nullptr);

        ~TabContentWidget() override;

        void appendValue();
    private:
        Ui::TabContentWidget *ui;
        QChartView* Chart;
        QLineSeries* LineSeries;


        void setupUi();

    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_TABCONTENTWIDGET_H
