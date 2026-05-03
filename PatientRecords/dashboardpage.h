#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVector>
#include "backend.h"

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(Appointments::AppointmentQueue* queue,
                           QVector<MedicalRecords::patient*>* patients,
                           int* doctorCount,
                           Financials::BillingManager* billing,
                           QWidget* parent = nullptr);

    void refresh();

private:
    QLabel* makeBadge(const QString& status);

    Appointments::AppointmentQueue*     apptQueue;
    QVector<MedicalRecords::patient*>*  patients;
    int*                                doctorCount;
    Financials::BillingManager*         billing;

    QTableWidget* apptTable;
    QLabel* totalPatientsVal;
    QLabel* apptBookedVal;
    QLabel* revenueVal;
    QLabel* doctorsVal;

signals:
    void requestBookAppointment();
    void requestNewBill();
};
