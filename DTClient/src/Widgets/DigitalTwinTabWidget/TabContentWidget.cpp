//
// Created by Moritz Herzog on 11.09.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TabContentWidget.h" resolved

#include "TabContentWidget.h"
#include "ui_TabContentWidget.h"



namespace DigitalTwin::Client {
    TabContentWidget::TabContentWidget(QWidget *parent) :
            QWidget(parent),
            ui(new Ui::TabContentWidget),
            InternalStatus(TabContentWidgetStatus::TableView){
        ui->setupUi(this);
        Chart = new QChartView(this);
        LineSeries = new QLineSeries(Chart);

        setupUi();
        makeConnection();
    }

    TabContentWidget::~TabContentWidget() {
        delete ui;
        delete Chart;
    }

    void TabContentWidget::setupUi() {
        ui->page->setLayout(new QHBoxLayout);
        ui->page->layout()->addWidget(Chart);

    }

    void TabContentWidget::appendValue() {

    }

    void TabContentWidget::makeConnection() {
        connect(ui->ToggleViewButton, SIGNAL(clicked(bool)), this, SLOT(toogleButtonClicked()));
    }

    void TabContentWidget::toogleButtonClicked() {
        if(InternalStatus==TableView) {
            InternalStatus = LineView;
            ui->ToggleViewButton->setText(tr("Toggle to Table"));
            ui->stackedWidget->widget(InternalStatus);
        }else if(InternalStatus == LineView) {
            InternalStatus = TableView;
            ui->ToggleViewButton->setText(tr("Toggle to Diagram"));
            ui->stackedWidget->widget(InternalStatus);
        }
    }
} // DigitalTwin::Client
