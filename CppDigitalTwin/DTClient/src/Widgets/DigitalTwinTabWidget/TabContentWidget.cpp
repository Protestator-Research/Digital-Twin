//
// Created by Moritz Herzog on 11.09.24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TabContentWidget.h" resolved

#include "TabContentWidget.h"
#include "../../ui_TabContentWidget.h"



namespace DigitalTwin::Client {
    TabContentWidget::TabContentWidget(QWidget *parent) :
            QWidget(parent), ui(new Ui::TabContentWidget) {
        ui->setupUi(this);
        Chart = new QChartView(this);
        LineSeries = new QLineSeries(Chart);

        setupUi();
    }

    TabContentWidget::~TabContentWidget() {
        delete ui;
        delete Chart;
        delete LineSeries;
    }

    void TabContentWidget::setupUi() {
        ui->page->setLayout(new QHBoxLayout);
        ui->page->layout()->addWidget(Chart);

    }

    void TabContentWidget::appendValue() {

    }
} // DigitalTwin::Client
