#include "dashboardpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>
#include <QDate>

DashboardPage::DashboardPage(Appointments::AppointmentQueue* queue,
                             QVector<MedicalRecords::patient*>* pts,
                             int* docCnt,
                             Financials::BillingManager* bill,
                             QWidget* parent)
    : QWidget(parent), apptQueue(queue), patients(pts),
      doctorCount(docCnt), billing(bill)
{
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget();
    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(32,28,32,28);
    mainLayout->setSpacing(24);

    auto* title = new QLabel("Dashboard");
    title->setObjectName("pageTitle");
    mainLayout->addWidget(title);

    auto* kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(16);

    totalPatientsVal = new QLabel("0");
    apptBookedVal    = new QLabel("0");
    revenueVal       = new QLabel("0");
    doctorsVal       = new QLabel("0");

    auto buildCard = [&](const QString& icon, QLabel* valLbl,
                         const QString& lbl, const QString& color) {
        auto* card = new QFrame();
        card->setObjectName("card");
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20,20,20,20);
        cl->setSpacing(8);
        auto* iconLbl = new QLabel(icon);
        iconLbl->setObjectName("kpiIcon");
        iconLbl->setStyleSheet(QString(
            "color: %1; background: %2; border-radius: 8px;"
            "padding: 8px; max-width: 44px; max-height: 44px;")
            .arg(color).arg(color + "22"));
        cl->addWidget(iconLbl);
        valLbl->setObjectName("kpiValue");
        cl->addWidget(valLbl);
        auto* lblW = new QLabel(lbl);
        lblW->setObjectName("kpiLabel");
        cl->addWidget(lblW);
        return card;
    };

    kpiRow->addWidget(buildCard("👥", totalPatientsVal, "Total Patients",      "#00B4D8"));
    kpiRow->addWidget(buildCard("📅", apptBookedVal,    "Today's Appointments","#10b981"));
    kpiRow->addWidget(buildCard("💲", revenueVal,       "Revenue (EGP)",       "#f59e0b"));
    kpiRow->addWidget(buildCard("🩺", doctorsVal,       "Doctors On Duty",     "#8b5cf6"));
    mainLayout->addLayout(kpiRow);

    auto* qaLabel = new QLabel("Quick Actions");
    qaLabel->setObjectName("sectionTitle");
    mainLayout->addWidget(qaLabel);

    auto* qaRow = new QHBoxLayout();
    qaRow->setSpacing(12);
    auto* bookBtn = new QPushButton("+ Book Appointment");
    bookBtn->setObjectName("primaryBtn");
    bookBtn->setFixedHeight(40);
    auto* billBtn = new QPushButton("  New Bill");
    billBtn->setObjectName("secondaryBtn");
    billBtn->setFixedHeight(40);
    qaRow->addWidget(bookBtn);
    qaRow->addWidget(billBtn);
    qaRow->addStretch();
    mainLayout->addLayout(qaRow);

    connect(bookBtn, &QPushButton::clicked, this, &DashboardPage::requestBookAppointment);
    connect(billBtn, &QPushButton::clicked, this, &DashboardPage::requestNewBill);

    auto* apptCard = new QFrame();
    apptCard->setObjectName("card");
    auto* apptLayout = new QVBoxLayout(apptCard);
    apptLayout->setContentsMargins(20,20,20,20);
    apptLayout->setSpacing(16);

    auto* apptTitle = new QLabel("Today's Appointments");
    apptTitle->setObjectName("sectionTitle");
    apptLayout->addWidget(apptTitle);

    apptTable = new QTableWidget();
    apptTable->setColumnCount(5);
    apptTable->setHorizontalHeaderLabels({"PATIENT NAME","DOCTOR","CLINIC","TIME","STATUS"});
    apptTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    apptTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    apptTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    apptTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    apptTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    apptTable->verticalHeader()->setVisible(false);
    apptTable->setEditTriggers(QTableWidget::NoEditTriggers);
    apptTable->setSelectionBehavior(QTableWidget::SelectRows);
    apptTable->setAlternatingRowColors(true);
    apptTable->setStyleSheet("QTableWidget { alternate-background-color: #fafafa; }");
    apptTable->setMinimumHeight(300);
    apptLayout->addWidget(apptTable);
    mainLayout->addWidget(apptCard);

    mainLayout->addStretch();
    scroll->setWidget(content);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0,0,0,0);
    outerLayout->addWidget(scroll);

    refresh();
}

QLabel* DashboardPage::makeBadge(const QString& status) {
    auto* badge = new QLabel(status);
    badge->setAlignment(Qt::AlignCenter);
    badge->setFixedSize(90, 24);
    badge->setStyleSheet("border-radius: 12px; font-size: 12px; font-weight: bold; padding: 2px 10px;");

    if (status == "Completed")
        badge->setStyleSheet(badge->styleSheet() + "background:#dcfce7; color:#16a34a;");
    else if (status == "Cancelled")
        badge->setStyleSheet(badge->styleSheet() + "background:#fee2e2; color:#dc2626;");
    else
        badge->setStyleSheet(badge->styleSheet() + "background:#dbeafe; color:#2563eb;");
    return badge;
}

void DashboardPage::refresh() {
    if (patients)
        totalPatientsVal->setText(QString::number(patients->size()));

    if (doctorCount)
        doctorsVal->setText(QString::number(*doctorCount));

    if (billing) {
        auto bills = billing->loadAllBills();
        double total = 0;
        for (auto& b : bills) total += b.totalAmount + b.insuranceCoverage;
        revenueVal->setText(QString::number((int)total));
    }

    apptTable->setRowCount(0);
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    auto& all = apptQueue->getAll();
    int booked = 0;
    for (auto& a : all) {
        if (QString::fromStdString(a.date) != today) continue;
        int row = apptTable->rowCount();
        apptTable->insertRow(row);
        apptTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(a.patient_name)));
        apptTable->setItem(row,1,new QTableWidgetItem(QString::fromStdString(a.doctor_name)));
        apptTable->setItem(row,2,new QTableWidgetItem(QString::fromStdString(a.clinic_name)));
        apptTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(a.time)));
        apptTable->setCellWidget(row,4,makeBadge(QString::fromStdString(a.status)));
        apptTable->setRowHeight(row,52);
        booked++;
    }

    apptBookedVal->setText(QString::number(booked));
}
