#include "financialpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QGridLayout>
#include <QDateTime>
#include <ctime>

FinancialPage::FinancialPage(QWidget* parent) : QWidget(parent) {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget();
    auto* mainLay = new QVBoxLayout(content);
    mainLay->setContentsMargins(32,28,32,28);
    mainLay->setSpacing(24);

    auto* title = new QLabel("Financial & Billing");
    title->setObjectName("pageTitle");
    mainLay->addWidget(title);

    auto* kpiRow = new QHBoxLayout();
    kpiRow->setSpacing(16);

    auto makeKpi = [&](const QString& lbl, QLabel*& valLbl, const QString& iconColor) {
        auto* card = new QFrame();
        card->setObjectName("card");
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20,20,20,20);
        cl->setSpacing(6);
        auto* lLbl = new QLabel(lbl);
        lLbl->setStyleSheet("color: #9ca3af; font-size:13px;");
        cl->addWidget(lLbl);
        valLbl = new QLabel("0 EGP");
        valLbl->setStyleSheet(QString("font-size:26px; font-weight:bold; color:%1;").arg(iconColor));
        cl->addWidget(valLbl);
        return card;
    };

    kpiRow->addWidget(makeKpi("Total Patient Paid",    totalPatientLbl,   "#0D1B2A"));
    kpiRow->addWidget(makeKpi("Total Insurance Coverage", totalInsuranceLbl, "#10b981"));
    kpiRow->addWidget(makeKpi("Net Hospital Revenue",  netRevenueLbl,     "#00B4D8"));
    mainLay->addLayout(kpiRow);

    auto* billCard = new QFrame();
    billCard->setObjectName("card");
    auto* billLay = new QVBoxLayout(billCard);
    billLay->setContentsMargins(24,24,24,24);
    billLay->setSpacing(16);

    auto* billTitle = new QLabel("New Bill");
    billTitle->setObjectName("sectionTitle");
    billLay->addWidget(billTitle);

    auto* formGrid = new QHBoxLayout();
    formGrid->setSpacing(24);

    auto* leftCol = new QVBoxLayout();
    leftCol->setSpacing(12);

    auto* pidLbl = new QLabel("Patient ID");
    pidLbl->setStyleSheet("font-weight:bold; color:#374151;");
    leftCol->addWidget(pidLbl);
    patientIdEdit = new QLineEdit();
    patientIdEdit->setPlaceholderText("Enter patient ID");
    patientIdEdit->setFixedHeight(40);
    leftCol->addWidget(patientIdEdit);

    auto* clinicLbl = new QLabel("Clinic");
    clinicLbl->setStyleSheet("font-weight:bold; color:#374151;");
    leftCol->addWidget(clinicLbl);
    clinicCombo = new QComboBox();
    clinicCombo->setFixedHeight(40);
    clinicCombo->addItem("Select clinic");
    clinicCombo->addItem("ENT          — 500 EGP");
    clinicCombo->addItem("Dermatology  — 600 EGP");
    clinicCombo->addItem("Cardiology   — 1,000 EGP");
    leftCol->addWidget(clinicCombo);
    connect(clinicCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FinancialPage::onClinicChanged);

    auto* insLbl = new QLabel("Insurance Provider");
    insLbl->setStyleSheet("font-weight:bold; color:#374151;");
    leftCol->addWidget(insLbl);
    insuranceCombo = new QComboBox();
    insuranceCombo->setFixedHeight(40);
    insuranceCombo->addItem("No insurance (0%)");
    insuranceCombo->addItem("AXA     — 80%");
    insuranceCombo->addItem("Metlife — 70%");
    insuranceCombo->addItem("Allianz — 90%");
    leftCol->addWidget(insuranceCombo);
    connect(insuranceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FinancialPage::onInsuranceChanged);

    auto* breakdownCard = new QFrame();
    breakdownCard->setStyleSheet("background:#f9fafb; border:1px solid #e5e7eb; border-radius:8px; padding:12px;");
    auto* bdLay = new QVBoxLayout(breakdownCard);
    bdLay->setSpacing(6);

    auto makeRow = [&](const QString& lbl, QLabel*& valLbl, bool bold = false) {
        auto* row = new QHBoxLayout();
        auto* l = new QLabel(lbl);
        l->setStyleSheet(bold ? "font-weight:bold; color:#0D1B2A;" : "color:#6b7280;");
        valLbl = new QLabel("—");
        valLbl->setStyleSheet(bold ? "font-weight:bold; font-size:16px; color:#0D1B2A;" : "color:#374151;");
        row->addWidget(l); row->addStretch(); row->addWidget(valLbl);
        bdLay->addLayout(row);
    };
    makeRow("Base Price:",       basePriceLabel);
    makeRow("Insurance Covers:", insuranceLabel);
    makeRow("Patient Pays:",     patientPaysLabel, true);
    leftCol->addWidget(breakdownCard);
    leftCol->addStretch();
    formGrid->addLayout(leftCol, 1);

    auto* rightCol = new QVBoxLayout();
    rightCol->setSpacing(12);

    auto* payTitle = new QLabel("Payment Details");
    payTitle->setStyleSheet("font-weight:bold; color:#374151; font-size:15px;");
    rightCol->addWidget(payTitle);

    auto addPayField = [&](const QString& lbl, QLineEdit*& edit, const QString& ph, bool pw = false) {
        auto* l = new QLabel(lbl);
        l->setStyleSheet("font-weight:bold; color:#374151;");
        rightCol->addWidget(l);
        edit = new QLineEdit();
        edit->setPlaceholderText(ph);
        edit->setFixedHeight(40);
        if (pw) edit->setEchoMode(QLineEdit::Password);
        rightCol->addWidget(edit);
    };

    addPayField("Card Holder Name", cardHolderEdit, "Enter name");
    addPayField("Card Number",      cardNumberEdit, "****************");
    auto* expRow = new QHBoxLayout();
    expRow->setSpacing(12);
    auto* expLay = new QVBoxLayout();
    auto* expLbl = new QLabel("Expiry");
    expLbl->setStyleSheet("font-weight:bold; color:#374151;");
    expiryEdit = new QLineEdit(); expiryEdit->setPlaceholderText("MM/YY"); expiryEdit->setFixedHeight(40);
    expLay->addWidget(expLbl); expLay->addWidget(expiryEdit);
    auto* cvvLay = new QVBoxLayout();
    auto* cvvLbl = new QLabel("CVV");
    cvvLbl->setStyleSheet("font-weight:bold; color:#374151;");
    cvvEdit = new QLineEdit(); cvvEdit->setPlaceholderText("***"); cvvEdit->setFixedHeight(40);
    cvvEdit->setEchoMode(QLineEdit::Password);
    cvvLay->addWidget(cvvLbl); cvvLay->addWidget(cvvEdit);
    expRow->addLayout(expLay); expRow->addLayout(cvvLay);
    rightCol->addLayout(expRow);

    rightCol->addStretch();

    auto* payBtn = new QPushButton("Process Payment");
    payBtn->setObjectName("primaryBtn");
    payBtn->setFixedHeight(48);
    rightCol->addWidget(payBtn);
    connect(payBtn, &QPushButton::clicked, this, &FinancialPage::processPayment);

    formGrid->addLayout(rightCol, 1);
    billLay->addLayout(formGrid);
    mainLay->addWidget(billCard);

    auto* tableCard = new QFrame();
    tableCard->setObjectName("card");
    auto* tableLay = new QVBoxLayout(tableCard);
    tableLay->setContentsMargins(20,20,20,20);
    tableLay->setSpacing(12);

    auto* tTitle = new QLabel("Recent Billing Records");
    tTitle->setObjectName("sectionTitle");
    tableLay->addWidget(tTitle);

    billsTable = new QTableWidget();
    billsTable->setColumnCount(5);
    billsTable->setHorizontalHeaderLabels({"BILL ID","PATIENT ID","SERVICE","AMOUNT (EGP)","DATE"});
    billsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    billsTable->verticalHeader()->setVisible(false);
    billsTable->setEditTriggers(QTableWidget::NoEditTriggers);
    billsTable->setMinimumHeight(220);
    tableLay->addWidget(billsTable);
    mainLay->addWidget(tableCard);

    mainLay->addStretch();
    scroll->setWidget(content);

    auto* outerLay = new QVBoxLayout(this);
    outerLay->setContentsMargins(0,0,0,0);
    outerLay->addWidget(scroll);

    refreshBillsTable();
}

void FinancialPage::onClinicChanged(int index) {
    QStringList clinics = {"", "ENT", "Dermatology", "Cardiology"};
    if (index <= 0 || index >= clinics.size()) { currentBasePrice = 0; return; }

    currentBasePrice = clinicSystem.getBasePrice(clinics[index].toStdString());

    onInsuranceChanged(insuranceCombo->currentIndex());
}

void FinancialPage::onInsuranceChanged(int index) {
    QStringList providers = {"None", "AXA", "Metlife", "Allianz"};
    if (index < 0 || index >= providers.size()) return;

    currentInsurancePct = insuranceSystem.getCoverage(providers[index].toStdString());

    double insCovers  = currentBasePrice * currentInsurancePct;
    double patientPay = currentBasePrice - insCovers;

    basePriceLabel->setText(QString::number(currentBasePrice, 'f', 0) + " EGP");
    insuranceLabel->setText(QString("-%1 EGP  (%2%)")
        .arg(insCovers, 0, 'f', 0)
        .arg((int)(currentInsurancePct * 100)));
    insuranceLabel->setStyleSheet("color: #16a34a;");
    patientPaysLabel->setText(QString::number(patientPay, 'f', 0) + " EGP");
}

void FinancialPage::processPayment() {
    if (patientIdEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Missing","Please enter a Patient ID."); return; }
    if (clinicCombo->currentIndex() <= 0) {
        QMessageBox::warning(this,"Missing","Please select a clinic."); return; }
    if (currentBasePrice <= 0) {
        QMessageBox::warning(this,"Missing","Please select a valid clinic."); return; }
    if (cardHolderEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Missing","Please enter the card holder name."); return; }
    if (cardNumberEdit->text().trimmed().length() < 8) {
        QMessageBox::warning(this,"Invalid Card","Card number must be at least 8 digits."); return; }
    if (cvvEdit->text().trimmed().length() < 3) {
        QMessageBox::warning(this,"Invalid CVV","CVV must be at least 3 digits."); return; }

    paymentTerminal.setCardDetails(
        cardNumberEdit->text().toStdString(),
        cardHolderEdit->text().toStdString(),
        expiryEdit->text().toStdString(),
        cvvEdit->text().toStdString());

    double insCovers  = currentBasePrice * currentInsurancePct;
    double patientPay = currentBasePrice - insCovers;

    bool ok = paymentTerminal.processTransaction(patientPay);
    if (!ok) { QMessageBox::critical(this,"Failed","Transaction failed. Check card details."); return; }

    static int billCounter = 1;
    QStringList clinics = {"","ENT","Dermatology","Cardiology"};
    Financials::Financialdata bill;
    bill.billId          = billCounter++;
    bill.PatientId       = patientIdEdit->text().toInt();
    bill.serviceDetails  = clinics[clinicCombo->currentIndex()].toStdString() + " Consultation";
    bill.totalAmount     = patientPay;
    bill.insuranceCoverage = insCovers;
    bill.issuedate       = time(nullptr);
    billingManager.saveBill(bill);

    QMessageBox::information(this,"Success",
        QString("Payment of %1 EGP processed successfully!").arg(patientPay,0,'f',0));

    patientIdEdit->clear(); cardHolderEdit->clear();
    cardNumberEdit->clear(); expiryEdit->clear(); cvvEdit->clear();
    clinicCombo->setCurrentIndex(0); insuranceCombo->setCurrentIndex(0);
    basePriceLabel->setText("—"); insuranceLabel->setText("—"); patientPaysLabel->setText("—");

    refreshBillsTable();
}

void FinancialPage::refreshBillsTable() {
    billsTable->setRowCount(0);
    auto bills = billingManager.loadAllBills();

    double totalPatient = 0, totalInsurance = 0;
    for (auto& b : bills) {
        int row = billsTable->rowCount();
        billsTable->insertRow(row);
        billsTable->setItem(row,0,new QTableWidgetItem(QString("B%1").arg(b.billId,3,10,QChar('0'))));
        billsTable->setItem(row,1,new QTableWidgetItem(QString("P%1").arg(b.PatientId)));
        billsTable->setItem(row,2,new QTableWidgetItem(QString::fromStdString(b.serviceDetails)));
        billsTable->setItem(row,3,new QTableWidgetItem(QString::number(b.totalAmount,'f',0)));
        QDateTime dt;
        dt.setSecsSinceEpoch(b.issuedate);
        billsTable->setItem(row,4,new QTableWidgetItem(dt.toString("yyyy-MM-dd")));
        billsTable->setRowHeight(row,48);
        totalPatient   += b.totalAmount;
        totalInsurance += b.insuranceCoverage;
    }

    totalPatientLbl->setText(QString::number(totalPatient,'f',0) + " EGP");
    totalInsuranceLbl->setText(QString::number(totalInsurance,'f',0) + " EGP");
    netRevenueLbl->setText(QString::number(totalPatient + totalInsurance,'f',0) + " EGP");
}
