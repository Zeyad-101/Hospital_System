#pragma once
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QVector>
#include "backend.h"

class PatientsPage : public QWidget {
    Q_OBJECT
public:
    PatientsPage(QVector<MedicalRecords::patient*>& patients,
                 Staff::Doctor** doctors, int docCount,
                 QWidget* parent = nullptr);

    void refreshPatientList();

private slots:
    void onPatientSelected(int row);
    void showAddRecordDialog();
    void showAddPatientDialog();
    void showEditPatientDialog();

private:
    void buildPatientPanel(MedicalRecords::patient* p);

    void refreshTimeline(MedicalRecords::patient* p);

    QVector<MedicalRecords::patient*>& patients;
    Staff::Doctor** doctors;
    int docCount;

    QListWidget* patientList;
    QWidget*     detailPanel;
    QVBoxLayout* detailLayout;

    QWidget*     timelineWidget   = nullptr;
    QVBoxLayout* timelineLayout   = nullptr;

    int currentPatientIndex = -1;
};
