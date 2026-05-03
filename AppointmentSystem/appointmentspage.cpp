#include "appointmentspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QLabel>

AppointmentsPage::AppointmentsPage(Staff::Doctor** docs, int cnt,
                                   Appointments::AppointmentQueue* q,
                                   QWidget* parent)
    : QWidget(parent), doctors(docs), docCount(cnt), queue(q)
{
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content  = new QWidget();
    auto* mainLay  = new QVBoxLayout(content);
    mainLay->setContentsMargins(32,28,32,28);
    mainLay->setSpacing(24);

    auto* title = new QLabel("Book Appointment");
    title->setObjectName("pageTitle");
    mainLay->addWidget(title);

    auto* card = new QFrame();
    card->setObjectName("card");
    auto* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(28,28,28,28);
    cardLay->setSpacing(16);

    auto addField = [&](const QString& lbl, QWidget* widget) {
        auto* l = new QLabel(lbl);
        l->setStyleSheet("font-weight:bold; color:#374151;");
        cardLay->addWidget(l);
        cardLay->addWidget(widget);
    };

    patientNameEdit = new QLineEdit();
    patientNameEdit->setPlaceholderText("Enter patient full name");
    patientNameEdit->setFixedHeight(40);
    addField("Patient Name", patientNameEdit);

    doctorCombo = new QComboBox();
    doctorCombo->setFixedHeight(40);
    doctorCombo->addItem("-- Select a doctor --");
    for (int i = 0; i < docCount; i++)
        doctorCombo->addItem(QString("[%1] %2 (%3)")
            .arg(doctors[i]->getId())
            .arg(QString::fromStdString(doctors[i]->name))
            .arg(QString::fromStdString(doctors[i]->spec)));
    addField("Doctor", doctorCombo);

    connect(doctorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AppointmentsPage::onDoctorSelected);

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setFixedHeight(40);
    addField("Date", dateEdit);

    timeEdit = new QTimeEdit(QTime(9,0));
    timeEdit->setDisplayFormat("hh:mm AP");
    timeEdit->setFixedHeight(40);
    addField("Time", timeEdit);

    clinicEdit = new QLineEdit();
    clinicEdit->setPlaceholderText("Auto-filled from selected doctor");
    clinicEdit->setReadOnly(true);
    clinicEdit->setFixedHeight(40);
    clinicEdit->setStyleSheet("background:#f3f4f6; color:#9ca3af;");
    addField("Clinic (auto)", clinicEdit);

    auto* checkBtn = new QPushButton("Check Availability");
    checkBtn->setObjectName("secondaryBtn");
    checkBtn->setFixedHeight(38);
    checkBtn->setFixedWidth(190);
    cardLay->addWidget(checkBtn);
    connect(checkBtn, &QPushButton::clicked, this, &AppointmentsPage::checkAvailability);

    availabilityLabel = new QLabel("");
    availabilityLabel->setVisible(false);
    cardLay->addWidget(availabilityLabel);

    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(12);
    auto* confirmBtn = new QPushButton("✓  Confirm Booking");
    confirmBtn->setObjectName("primaryBtn");
    confirmBtn->setFixedHeight(44);
    auto* cancelBtn  = new QPushButton("Clear");
    cancelBtn->setObjectName("secondaryBtn");
    cancelBtn->setFixedHeight(44);
    btnRow->addWidget(confirmBtn);
    btnRow->addWidget(cancelBtn);
    btnRow->addStretch();
    cardLay->addLayout(btnRow);

    connect(confirmBtn, &QPushButton::clicked, this, &AppointmentsPage::confirmBooking);
    connect(cancelBtn,  &QPushButton::clicked, this, &AppointmentsPage::cancelBooking);

    mainLay->addWidget(card);

    auto* docCard = new QFrame();
    docCard->setObjectName("card");
    auto* docLay  = new QVBoxLayout(docCard);
    docLay->setContentsMargins(20,20,20,20);
    docLay->setSpacing(12);
    auto* docTitle = new QLabel("Selected Doctor — Today's Schedule");
    docTitle->setObjectName("sectionTitle");
    docLay->addWidget(docTitle);

    doctorApptTable = new QTableWidget();
    doctorApptTable->setColumnCount(4);
    doctorApptTable->setHorizontalHeaderLabels({"PATIENT","TIME","CLINIC","STATUS"});
    doctorApptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    doctorApptTable->verticalHeader()->setVisible(false);
    doctorApptTable->setEditTriggers(QTableWidget::NoEditTriggers);
    doctorApptTable->setMinimumHeight(180);
    docLay->addWidget(doctorApptTable);
    mainLay->addWidget(docCard);

    auto* allCard = new QFrame();
    allCard->setObjectName("card");
    auto* allLay  = new QVBoxLayout(allCard);
    allLay->setContentsMargins(20,20,20,20);
    allLay->setSpacing(12);
    auto* allTitle = new QLabel("All Appointments");
    allTitle->setObjectName("sectionTitle");
    allLay->addWidget(allTitle);

    allApptTable = new QTableWidget();
    allApptTable->setColumnCount(6);
    allApptTable->setHorizontalHeaderLabels({"PATIENT","DOCTOR","CLINIC","DATE","TIME","STATUS"});
    allApptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    allApptTable->verticalHeader()->setVisible(false);
    allApptTable->setEditTriggers(QTableWidget::NoEditTriggers);
    allApptTable->setAlternatingRowColors(true);
    allApptTable->setMinimumHeight(220);
    allLay->addWidget(allApptTable);
    mainLay->addWidget(allCard);

    mainLay->addStretch();
    scroll->setWidget(content);

    auto* outerLay = new QVBoxLayout(this);
    outerLay->setContentsMargins(0,0,0,0);
    outerLay->addWidget(scroll);

    refreshAllAppointmentsTable();
}

void AppointmentsPage::onDoctorSelected(int index) {
    if (index <= 0 || index > docCount) { clinicEdit->clear(); return; }

    clinicEdit->setText(QString::fromStdString(doctors[index-1]->spec));

    refreshDoctorAppointments();
}

void AppointmentsPage::checkAvailability() {
    int idx = doctorCombo->currentIndex();
    if (idx <= 0) {
        QMessageBox::warning(this, "Select Doctor", "Please select a doctor first."); return;
    }

    QString time = timeEdit->time().toString("hh:mm AP");
    bool busy = queue->isDoctorBusy(doctors[idx-1]->name, time.toStdString());

    availabilityLabel->setVisible(true);
    if (busy) {
        availabilityLabel->setText("✗  This time slot is already booked for this doctor");
        availabilityLabel->setStyleSheet("color:#dc2626; font-weight:bold; padding:6px;");
    } else {
        availabilityLabel->setText("✓  Time slot is available — ready to book");
        availabilityLabel->setStyleSheet("color:#16a34a; font-weight:bold; padding:6px;");
    }
}

void AppointmentsPage::confirmBooking() {
    if (patientNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Missing Info","Please enter a patient name."); return;
    }
    int idx = doctorCombo->currentIndex();
    if (idx <= 0) {
        QMessageBox::warning(this,"Missing Info","Please select a doctor."); return;
    }
    QString patient = patientNameEdit->text().trimmed();
    QString doctor  = QString::fromStdString(doctors[idx-1]->name);
    QString date    = dateEdit->date().toString("yyyy-MM-dd");
    QString time    = timeEdit->time().toString("hh:mm AP");
    QString clinic  = clinicEdit->text();

    bool ok = queue->book(patient.toStdString(), doctor.toStdString(),
                          date.toStdString(), time.toStdString(), clinic.toStdString());
    if (ok) {
        QMessageBox::information(this,"Success",
            QString("Appointment booked!\n\nPatient: %1\nDoctor: %2\nDate: %3  Time: %4")
            .arg(patient).arg(doctor).arg(date).arg(time));
        patientNameEdit->clear();
        doctorCombo->setCurrentIndex(0);
        availabilityLabel->setVisible(false);

        refreshDoctorAppointments();
        refreshAllAppointmentsTable();

        emit appointmentBooked();
    } else {
        QMessageBox::warning(this,"Conflict",
            "This doctor already has an appointment at that time.\nPlease choose a different time.");
    }
}

void AppointmentsPage::cancelBooking() {
    patientNameEdit->clear();
    doctorCombo->setCurrentIndex(0);
    clinicEdit->clear();
    availabilityLabel->setVisible(false);
    doctorApptTable->setRowCount(0);
}

void AppointmentsPage::refreshAll() {
    refreshDoctorAppointments();
    refreshAllAppointmentsTable();
}

void AppointmentsPage::refreshDoctorAppointments() {
    doctorApptTable->setRowCount(0);
    int idx = doctorCombo->currentIndex();
    if (idx <= 0) return;

    string docName = doctors[idx-1]->name;
    QString today  = QDate::currentDate().toString("yyyy-MM-dd");
    for (auto& a : queue->getAll()) {
        if (a.doctor_name != docName) continue;
        if (QString::fromStdString(a.date) != today) continue;
        int row = doctorApptTable->rowCount();
        doctorApptTable->insertRow(row);
        doctorApptTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(a.patient_name)));
        doctorApptTable->setItem(row,1,new QTableWidgetItem(QString::fromStdString(a.time)));
        doctorApptTable->setItem(row,2,new QTableWidgetItem(QString::fromStdString(a.clinic_name)));
        doctorApptTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(a.status)));
        doctorApptTable->setRowHeight(row,44);
    }
}

void AppointmentsPage::refreshAllAppointmentsTable() {
    allApptTable->setRowCount(0);
    for (auto& a : queue->getAll()) {
        int row = allApptTable->rowCount();
        allApptTable->insertRow(row);
        allApptTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(a.patient_name)));
        allApptTable->setItem(row,1,new QTableWidgetItem(QString::fromStdString(a.doctor_name)));
        allApptTable->setItem(row,2,new QTableWidgetItem(QString::fromStdString(a.clinic_name)));
        allApptTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(a.date)));
        allApptTable->setItem(row,4,new QTableWidgetItem(QString::fromStdString(a.time)));

        auto* statusItem = new QTableWidgetItem(QString::fromStdString(a.status));
        if (a.status == "Cancelled")
            statusItem->setForeground(QColor("#dc2626"));
        else if (a.status == "Completed")
            statusItem->setForeground(QColor("#16a34a"));
        else
            statusItem->setForeground(QColor("#2563eb"));
        allApptTable->setItem(row,5,statusItem);
        allApptTable->setRowHeight(row,44);
    }
}
