//
// Created by Moritz Herzog on 11.09.24.
//

#ifndef DIGITALTWIN_TABCONTENTWIDGET_H
#define DIGITALTWIN_TABCONTENTWIDGET_H

#include <QWidget>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QChartView>
#include <QLineSeries>

namespace DigitalTwin::Client {
    QT_BEGIN_NAMESPACE
    namespace Ui { class TabContentWidget; }
    QT_END_NAMESPACE

    class TabContentWidget : public QWidget {
    Q_OBJECT

        enum TabContentWidgetStatus {
            LineView = 0,
            TableView
        };

    public:
        explicit TabContentWidget(QWidget *parent = nullptr);
        ~TabContentWidget() override;

        void makeConnection();

        void appendValue();
    private:
        Ui::TabContentWidget *ui;
        QChartView* Chart;
        QLineSeries* LineSeries;

        TabContentWidgetStatus InternalStatus;


        void setupUi();

    private slots:
        void toogleButtonClicked();
    };
} // DigitalTwin::Client

#endif //DIGITALTWIN_TABCONTENTWIDGET_H
