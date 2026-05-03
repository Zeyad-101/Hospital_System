#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include "backend.h"

class DashboardPage;
class AppointmentsPage;
class PatientsPage;
class FinancialPage;
class SchedulePage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow();

private slots:
    void switchPage(int index);

private:
    void buildSidebar();

    void setupStyle();

    QWidget*        centralWidget;
    QWidget*        sidebar;
    QStackedWidget* stack;

    QVector<QPushButton*> navButtons;

    Staff::Doctor**                   hospitalDoctors = nullptr;
    int                               doctorCount     = 0;

    Appointments::AppointmentQueue*   apptQueue       = nullptr;

    QVector<MedicalRecords::patient*> patients;

    Financials::BillingManager*       billingManager  = nullptr;

    DashboardPage*    dashPage;
    AppointmentsPage* apptPage;
    PatientsPage*     patPage;
    FinancialPage*    finPage;
    SchedulePage*     schedPage;
};
