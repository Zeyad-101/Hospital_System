#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QTableWidget>
#include "backend.h"

class FinancialPage : public QWidget {
    Q_OBJECT
public:
    explicit FinancialPage(QWidget* parent = nullptr);

private slots:
    void onClinicChanged(int index);

    void onInsuranceChanged(int index);

    void processPayment();

private:
    void refreshBillsTable();

    Financials::Insurance     insuranceSystem;
    Financials::ClinicPricing clinicSystem;
    Financials::BillingManager billingManager;
    Financials::PaymentProcess paymentTerminal;

    QLineEdit* patientIdEdit;
    QComboBox* clinicCombo;
    QComboBox* insuranceCombo;
    QLabel*    basePriceLabel;
    QLabel*    insuranceLabel;
    QLabel*    patientPaysLabel;
    QLineEdit* cardHolderEdit;
    QLineEdit* cardNumberEdit;
    QLineEdit* expiryEdit;
    QLineEdit* cvvEdit;
    QTableWidget* billsTable;

    QLabel* totalPatientLbl;
    QLabel* totalInsuranceLbl;
    QLabel* netRevenueLbl;

    double currentBasePrice    = 0.0;
    double currentInsurancePct = 0.0;
};
