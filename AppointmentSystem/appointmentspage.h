#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLabel>
#include <QTableWidget>
#include "backend.h"

class AppointmentsPage : public QWidget {
    Q_OBJECT
public:
    AppointmentsPage(Staff::Doctor** doctors, int docCount,
                     Appointments::AppointmentQueue* queue,
                     QWidget* parent = nullptr);

    void refreshAll();

signals:
    void appointmentBooked();

private slots:
    void onDoctorSelected(int index);
    void checkAvailability();
    void confirmBooking();
    void cancelBooking();

private:
    void refreshDoctorAppointments();

    void refreshAllAppointmentsTable();

    Staff::Doctor**                  doctors;
    int                              docCount;
    Appointments::AppointmentQueue*  queue;

    QLineEdit*    patientNameEdit;
    QComboBox*    doctorCombo;
    QDateEdit*    dateEdit;
    QTimeEdit*    timeEdit;
    QLineEdit*    clinicEdit;
    QLabel*       availabilityLabel;
    QTableWidget* doctorApptTable;
    QTableWidget* allApptTable;
};
