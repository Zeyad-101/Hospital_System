#include "patientspage.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDateEdit>
#include <QDialog>
#include <QMessageBox>
#include <QGridLayout>
#include <QCheckBox>
#include <fstream>

using namespace std;

namespace {
void showStyledMessage(QWidget* parent, const QString& title, const QString& text, QMessageBox::Icon icon) {
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(icon);
    msgBox.setStyleSheet(R"(
            QMessageBox { background-color: #ffffff; }
            QLabel { color: #000000; }
            QPushButton { background-color: #e2e8f0; color: #000000; padding: 5px 15px; border-radius: 3px; }
            QPushButton:hover { background-color: #cbd5e1; }
        )");
    msgBox.exec();
}

void savePatientsToFile(const QVector<MedicalRecords::patient*>& patients, const string& filepath) {
    ofstream outFile(filepath);
    if (!outFile.is_open()) return;

    for (auto* p : patients) {
        outFile << p->get_p_id() << ","
                << p->get_name() << ","
                << p->get_age() << ","
                << p->get_insurance() << "\n";
    }
    outFile.close();
}

void saveMedicalRecordsToFile(const QVector<MedicalRecords::patient*>& patients, const string& filepath) {
    ofstream outFile(filepath);
    if (!outFile.is_open()) return;

    for (auto* p : patients) {
        for (auto* rec : p->get_history()) {
            outFile << rec->get_record_id() << ","
                    << p->get_p_id() << ","
                    << rec->get_date() << ","
                    << rec->get_clinic() << ","
                    << rec->get_diagnosis() << ","
                    << rec->get_doctor_id() << "\n";
        }
    }
    outFile.close();
}
}

PatientsPage::PatientsPage(QVector<MedicalRecords::patient*>& pts,
                           Staff::Doctor** docs, int cnt,
                           QWidget* parent)
    : QWidget(parent), patients(pts), doctors(docs), docCount(cnt)
{
    auto* mainLay = new QHBoxLayout(this);
    mainLay->setContentsMargins(0,0,0,0);
    mainLay->setSpacing(0);

    auto* leftPanel = new QWidget();
    leftPanel->setFixedWidth(290);
    leftPanel->setStyleSheet("background:#ffffff; border-right:1px solid #e5e7eb;");
    auto* leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(0,0,0,0);
    leftLay->setSpacing(0);

    auto* topBar = new QWidget();
    topBar->setStyleSheet("background:#ffffff;");
    auto* topLay = new QVBoxLayout(topBar);
    topLay->setContentsMargins(12,12,12,8);
    topLay->setSpacing(8);

    auto* titleLbl = new QLabel("Patient Medical Records");
    titleLbl->setObjectName("pageTitle");
    titleLbl->setWordWrap(true);
    topLay->addWidget(titleLbl);

    auto* searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search patients...");
    searchEdit->setFixedHeight(36);
    topLay->addWidget(searchEdit);

    auto* addPatBtn = new QPushButton("+ Add New Patient");
    addPatBtn->setObjectName("primaryBtn");
    addPatBtn->setFixedHeight(36);
    topLay->addWidget(addPatBtn);
    connect(addPatBtn, &QPushButton::clicked, this, &PatientsPage::showAddPatientDialog);
    addPatBtn->setStyleSheet(R"(
    QPushButton {
        background-color: #00B4D8;
        color: #ffffff;
        border: none;
        border-radius: 5px;
        padding: 8px 16px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #0096b4;
    }
)");
    leftLay->addWidget(topBar);

    patientList = new QListWidget();
    patientList->setFrameShape(QFrame::NoFrame);
    patientList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    patientList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    patientList->setUniformItemSizes(false);
    patientList->setSpacing(1);
    patientList->setStyleSheet(R"(
    QListWidget { border: none; outline: 0; background-color: #ffffff; color: #000000; }
    QListWidget::item { padding: 10px 14px; border-bottom: 1px solid #f3f4f6; color: #000000; }
    QListWidget::item:selected { background: #e0f7fa; border-left: 3px solid #00B4D8; color: #000000; }
    QListWidget::item:hover { background: #f0fdff; color: #000000; }
)");
    leftLay->addWidget(patientList, 1);
    mainLay->addWidget(leftPanel);

    connect(searchEdit, &QLineEdit::textChanged, [this](const QString& text) {
        for (int i = 0; i < patientList->count(); i++) {
            auto* item = patientList->item(i);
            item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
        }
    });

    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background:#f8fafc;");

    detailPanel = new QWidget();
    detailPanel->setStyleSheet("background:#f8fafc;");
    detailLayout = new QVBoxLayout(detailPanel);
    detailLayout->setContentsMargins(28,28,28,28);
    detailLayout->setSpacing(20);

    auto* placeholder = new QLabel("← Select a patient from the list");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color:#9ca3af; font-size:16px;");
    detailLayout->addWidget(placeholder);
    detailLayout->addStretch();

    scroll->setWidget(detailPanel);
    mainLay->addWidget(scroll, 1);

    connect(patientList, &QListWidget::currentRowChanged,
            this, &PatientsPage::onPatientSelected);

    refreshPatientList();
}

void PatientsPage::refreshPatientList() {
    int prevRow = patientList->currentRow();

    patientList->clear();
    for (auto* p : patients) {
        QString text = QString::fromStdString(p->get_name())
        + "\n"
            + QString::fromStdString(p->get_p_id());
        auto* item = new QListWidgetItem(text);
        item->setSizeHint(QSize(patientList->width(), 58));
        patientList->addItem(item);
    }
    if (prevRow >= 0 && prevRow < patientList->count())
        patientList->setCurrentRow(prevRow);
}

void PatientsPage::onPatientSelected(int row) {
    if (row < 0 || row >= patients.size()) return;

    currentPatientIndex = row;
    buildPatientPanel(patients[row]);
}

void PatientsPage::buildPatientPanel(MedicalRecords::patient* p) {
    while (QLayoutItem* item = detailLayout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    timelineWidget = nullptr;
    timelineLayout = nullptr;

    auto* infoCard = new QFrame();
    infoCard->setObjectName("card");
    auto* grid = new QGridLayout(infoCard);
    grid->setContentsMargins(20,16,20,16);
    grid->setSpacing(16);

    auto addInfo = [&](int col, const QString& label, const QString& value) {
        auto* lbl = new QLabel(label);
        lbl->setStyleSheet("color:#9ca3af; font-size:11px; text-transform:uppercase;");
        auto* val = new QLabel(value);
        val->setStyleSheet("font-weight:bold; font-size:14px; color:#0D1B2A;");
        grid->addWidget(lbl, 0, col);
        grid->addWidget(val, 1, col);
    };

    addInfo(0, "Patient Name", QString::fromStdString(p->get_name()));
    addInfo(1, "Patient ID",   QString::fromStdString(p->get_p_id()));
    addInfo(2, "Age",          QString::number(p->get_age()) + " years");
    addInfo(3, "Insurance",    QString::fromStdString(p->get_insurance()));
    detailLayout->addWidget(infoCard);

    auto* btnFrame = new QFrame();
    btnFrame->setObjectName("card");
    auto* btnLay = new QHBoxLayout(btnFrame);
    btnLay->setContentsMargins(16,12,16,12);
    btnLay->setSpacing(10);

    auto* addRecBtn = new QPushButton("+ Add Medical Record");
    addRecBtn->setObjectName("primaryBtn");
    addRecBtn->setFixedHeight(40);
    addRecBtn->setMinimumWidth(180);

    addRecBtn->setStyleSheet(R"(
    QPushButton {
        background-color: #00B4D8;
        color: #ffffff;
        border: none;
        border-radius: 5px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #0096b4;
    }
)");

    connect(addRecBtn, &QPushButton::clicked, this, &PatientsPage::showAddRecordDialog);

    auto* editBtn = new QPushButton("✎  Edit Patient Info");
    editBtn->setObjectName("secondaryBtn");
    editBtn->setFixedHeight(40);
    editBtn->setMinimumWidth(160);
    connect(editBtn, &QPushButton::clicked, this, &PatientsPage::showEditPatientDialog);

    btnLay->addWidget(addRecBtn);
    btnLay->addWidget(editBtn);
    btnLay->addStretch();
    detailLayout->addWidget(btnFrame);

    auto* recTitle = new QLabel("Medical Records Timeline");
    recTitle->setObjectName("sectionTitle");
    detailLayout->addWidget(recTitle);

    timelineWidget = new QWidget();
    timelineLayout = new QVBoxLayout(timelineWidget);
    timelineLayout->setContentsMargins(0,0,0,0);
    timelineLayout->setSpacing(0);
    refreshTimeline(p);
    detailLayout->addWidget(timelineWidget);
    detailLayout->addStretch();
}

void PatientsPage::refreshTimeline(MedicalRecords::patient* p) {
    if (!timelineLayout) return;

    while (QLayoutItem* item = timelineLayout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    if (p->get_history().empty()) {
        auto* empty = new QLabel("No medical records yet. Click '+ Add Medical Record' to add one.");
        empty->setStyleSheet("color:#9ca3af; padding:20px;");
        empty->setWordWrap(true);
        timelineLayout->addWidget(empty);
        return;
    }
    for (auto* rec : p->get_history()) {
        auto* row = new QHBoxLayout();
        row->setSpacing(16);

        auto* dotCol = new QVBoxLayout();
        auto* dot = new QLabel("●");
        dot->setStyleSheet("color:#00B4D8; font-size:14px;");
        dot->setFixedWidth(20);
        dotCol->addWidget(dot);
        auto* line = new QWidget();
        line->setFixedWidth(2);
        line->setStyleSheet("background:#e5e7eb;");
        line->setMinimumHeight(20);
        dotCol->addWidget(line);
        dotCol->addStretch();
        row->addLayout(dotCol);

        auto* card = new QFrame();
        card->setObjectName("card");
        card->setStyleSheet("QFrame#card { margin-bottom:12px; }");
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(16,14,16,14);
        cl->setSpacing(6);

        auto* headerRow = new QHBoxLayout();
        auto* dateLbl = new QLabel(QString::fromStdString(rec->get_date()));
        dateLbl->setStyleSheet("font-weight:bold; color:#374151;");
        headerRow->addWidget(dateLbl);
        headerRow->addStretch();

        QString clinic = QString::fromStdString(rec->get_clinic());
        auto* badge = new QLabel(clinic);
        QString bs = clinic == "Cardiology"  ? "background:#fee2e2; color:#dc2626;" :
                         clinic == "ENT"         ? "background:#dbeafe; color:#2563eb;" :
                         "background:#f3e8ff; color:#7c3aed;";
        badge->setStyleSheet("border-radius:10px; padding:2px 10px; font-size:12px;" + bs);
        headerRow->addWidget(badge);
        cl->addLayout(headerRow);

        string docName = "Unknown";
        for (int i = 0; i < docCount; i++)
            if (doctors[i]->getId() == rec->get_doctor_id())
                docName = doctors[i]->name;

        auto* drLbl = new QLabel("Doctor: " + QString::fromStdString(docName));
        drLbl->setStyleSheet("color:#6b7280; font-size:13px;");
        cl->addWidget(drLbl);

        auto* recIdLbl = new QLabel("Record ID: " + QString::fromStdString(rec->get_record_id()));
        recIdLbl->setStyleSheet("color:#9ca3af; font-size:12px;");
        cl->addWidget(recIdLbl);

        auto* diagLbl = new QLabel(QString::fromStdString(rec->get_diagnosis()));
        diagLbl->setStyleSheet("color:#374151;");
        diagLbl->setWordWrap(true);
        cl->addWidget(diagLbl);

        row->addWidget(card, 1);
        timelineLayout->addLayout(row);
    }
}

void PatientsPage::showAddPatientDialog() {
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Add New Patient");
    dlg->setFixedWidth(460);
    dlg->setModal(true);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(14);

    auto* title = new QLabel("Add New Patient");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#0D1B2A;");
    lay->addWidget(title);

    auto addField = [&](const QString& lbl, QWidget* w) {
        lay->addWidget(new QLabel(lbl));
        lay->addWidget(w);
    };

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Full name");
    nameEdit->setFixedHeight(38);
    addField("Patient Name *", nameEdit);

    auto* idEdit = new QLineEdit();
    idEdit->setPlaceholderText("e.g. P-105");
    idEdit->setFixedHeight(38);
    addField("Patient ID *", idEdit);

    auto* ageSpin = new QSpinBox();
    ageSpin->setRange(0, 130);
    ageSpin->setValue(25);
    ageSpin->setFixedHeight(38);
    addField("Age", ageSpin);

    auto* insCheck = new QCheckBox("Has Insurance");
    lay->addWidget(insCheck);

    auto* insCombo = new QComboBox();
    insCombo->setFixedHeight(38);
    insCombo->addItems({"AXA", "Metlife", "Allianz"});
    insCombo->setEnabled(false);
    lay->addWidget(insCombo);
    connect(insCheck, &QCheckBox::toggled, insCombo, &QComboBox::setEnabled);

    auto* btnRow = new QHBoxLayout();
    auto* saveBtn   = new QPushButton("Add Patient");
    saveBtn->setObjectName("primaryBtn");
    saveBtn->setFixedHeight(40);
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("secondaryBtn");
    cancelBtn->setFixedHeight(40);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(cancelBtn);
    lay->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().trimmed().isEmpty()) {
            showStyledMessage(dlg, "Missing", "Please enter a patient name.", QMessageBox::Warning);
            return;
        }
        if (idEdit->text().trimmed().isEmpty()) {
            showStyledMessage(dlg, "Missing", "Please enter a patient ID.", QMessageBox::Warning);
            return;
        }
        for (auto* p : patients) {
            if (QString::fromStdString(p->get_p_id()) == idEdit->text().trimmed()) {
                showStyledMessage(dlg, "Duplicate", "A patient with this ID already exists.", QMessageBox::Warning);
                return;
            }
        }

        bool hasIns = insCheck->isChecked();
        string insName = hasIns ? insCombo->currentText().toStdString() : "None";
        auto* newPat = new MedicalRecords::patient(
            nameEdit->text().trimmed().toStdString(),
            idEdit->text().trimmed().toStdString(),
            ageSpin->value(),
            hasIns, insName);

        patients.append(newPat);

        savePatientsToFile(patients, "patient.txt");

        refreshPatientList();
        patientList->setCurrentRow(patients.size() - 1);
        dlg->accept();
        showStyledMessage(this, "Added", nameEdit->text().trimmed() + " has been added successfully.", QMessageBox::Information);
    });

    dlg->exec();
}

void PatientsPage::showEditPatientDialog() {
    if (currentPatientIndex < 0) return;
    auto* p = patients[currentPatientIndex];

    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Edit Patient Info");
    dlg->setFixedWidth(460);
    dlg->setModal(true);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(14);

    auto* title = new QLabel("Edit Patient Information");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#0D1B2A;");
    lay->addWidget(title);

    auto* nameEdit = new QLineEdit(QString::fromStdString(p->get_name()));
    nameEdit->setFixedHeight(38);
    lay->addWidget(new QLabel("Patient Name"));
    lay->addWidget(nameEdit);

    auto* idLbl = new QLineEdit(QString::fromStdString(p->get_p_id()));
    idLbl->setReadOnly(true);
    idLbl->setStyleSheet("background:#f3f4f6; color:#9ca3af;");
    idLbl->setFixedHeight(38);
    lay->addWidget(new QLabel("Patient ID (read-only)"));
    lay->addWidget(idLbl);

    auto* ageSpin = new QSpinBox();
    ageSpin->setRange(0, 130);
    ageSpin->setValue(p->get_age());
    ageSpin->setFixedHeight(38);
    lay->addWidget(new QLabel("Age"));
    lay->addWidget(ageSpin);

    QString currentIns = QString::fromStdString(p->get_insurance());
    auto* insCheck = new QCheckBox("Has Insurance");
    insCheck->setChecked(currentIns != "None" && !currentIns.isEmpty());
    lay->addWidget(insCheck);

    auto* insCombo = new QComboBox();
    insCombo->setFixedHeight(38);
    insCombo->addItems({"AXA", "Metlife", "Allianz"});
    insCombo->setEnabled(insCheck->isChecked());
    if (insCombo->findText(currentIns) >= 0)
        insCombo->setCurrentText(currentIns);
    lay->addWidget(insCombo);
    connect(insCheck, &QCheckBox::toggled, insCombo, &QComboBox::setEnabled);

    auto* btnRow = new QHBoxLayout();
    auto* saveBtn   = new QPushButton("Save Changes");
    saveBtn->setObjectName("primaryBtn");
    saveBtn->setFixedHeight(40);
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("secondaryBtn");
    cancelBtn->setFixedHeight(40);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(cancelBtn);
    lay->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (nameEdit->text().trimmed().isEmpty()) {
            showStyledMessage(dlg, "Missing", "Name cannot be empty.", QMessageBox::Warning);
            return;
        }
        bool hasIns = insCheck->isChecked();
        string insName = hasIns ? insCombo->currentText().toStdString() : "None";

        auto oldRecs = p->get_history();
        auto* updated = new MedicalRecords::patient(
            nameEdit->text().trimmed().toStdString(),
            p->get_p_id(),
            ageSpin->value(),
            hasIns, insName);

        for (auto* r : oldRecs) updated->add_record(r);

        while (!p->get_history().empty()) p->get_history().pop_back();

        patients[currentPatientIndex] = updated;
        delete p;

        savePatientsToFile(patients, "patient.txt");

        refreshPatientList();
        buildPatientPanel(updated);
        dlg->accept();
        showStyledMessage(this, "Updated", "Patient information updated successfully.", QMessageBox::Information);
    });

    dlg->exec();
}

void PatientsPage::showAddRecordDialog() {
    if (currentPatientIndex < 0) return;
    auto* p = patients[currentPatientIndex];

    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Add Medical Record");
    dlg->setFixedWidth(480);
    dlg->setModal(true);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(14);

    auto* title = new QLabel("Add New Medical Record");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#0D1B2A;");
    lay->addWidget(title);

    auto* dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setFixedHeight(38);
    lay->addWidget(new QLabel("Date"));
    lay->addWidget(dateEdit);

    auto* clinicCombo = new QComboBox();
    clinicCombo->setFixedHeight(38);
    clinicCombo->addItems({"ENT","Dermatology","Cardiology"});
    lay->addWidget(new QLabel("Clinic"));
    lay->addWidget(clinicCombo);

    auto* docCombo = new QComboBox();
    docCombo->setFixedHeight(38);
    for (int i = 0; i < docCount; i++)
        docCombo->addItem(QString("[%1] %2 — %3")
                              .arg(doctors[i]->getId())
                              .arg(QString::fromStdString(doctors[i]->name))
                              .arg(QString::fromStdString(doctors[i]->spec)));
    lay->addWidget(new QLabel("Doctor"));
    lay->addWidget(docCombo);

    auto* diagEdit = new QTextEdit();
    diagEdit->setPlaceholderText("Enter diagnosis and treatment notes...");
    diagEdit->setFixedHeight(100);
    lay->addWidget(new QLabel("Diagnosis / Notes"));
    lay->addWidget(diagEdit);

    auto* btnRow = new QHBoxLayout();
    auto* saveBtn   = new QPushButton("Save Record");
    saveBtn->setObjectName("primaryBtn");
    saveBtn->setFixedHeight(40);
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("secondaryBtn");
    cancelBtn->setFixedHeight(40);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(cancelBtn);
    lay->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (diagEdit->toPlainText().trimmed().isEmpty()) {
            showStyledMessage(dlg, "Missing", "Please enter a diagnosis.", QMessageBox::Warning);
            return;
        }

        static int recCounter = 200;

        int docId = docCount > 0 ? doctors[docCombo->currentIndex()]->getId() : 0;

        auto* rec = new MedicalRecords::med_rec(
            p->get_name(), p->get_p_id(), p->get_age(),
            "REC-" + to_string(++recCounter),
            dateEdit->date().toString("yyyy-MM-dd").toStdString(),
            clinicCombo->currentText().toStdString(),
            diagEdit->toPlainText().trimmed().toStdString(),
            docId);

        p->add_record(rec);

        saveMedicalRecordsToFile(patients, "mr.txt");

        refreshTimeline(p);
        dlg->accept();
        showStyledMessage(this, "Saved", "Medical record added successfully.", QMessageBox::Information);
    });

    dlg->exec();
}
