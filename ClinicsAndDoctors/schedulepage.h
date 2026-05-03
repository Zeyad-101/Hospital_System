#pragma once
#include <QWidget>
#include <QMap>
#include <QVector>
#include <QVBoxLayout>
#include <QListWidget>
#include "backend.h"

class SchedulePage : public QWidget {
    Q_OBJECT
public:
    SchedulePage(Staff::Doctor** doctors, int docCount, QWidget* parent = nullptr);

private slots:
    void showAddDoctorDialog();
    void showRemoveDoctorDialog();

private:
    void buildScheduleGrid();

    void refreshGrid();

    QWidget* makeDoctorCard(Staff::Doctor* doc);

    Staff::Doctor** doctors;
    int             docCount;

    QMap<QString, QVector<Staff::Doctor*>> schedule;
    QStringList days = {"Friday","Saturday","Sunday","Monday","Tuesday","Wednesday","Thursday"};

    QMap<QString, QVBoxLayout*> dayLayouts;
    QListWidget* availDoctorsList;
};
