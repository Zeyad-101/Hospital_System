#include "schedulepage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QGridLayout>

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
}

SchedulePage::SchedulePage(Staff::Doctor** docs, int cnt, QWidget* parent)
    : QWidget(parent), doctors(docs), docCount(cnt)
{
    if (docCount > 0) {
        for (int i = 0; i < docCount; i++) {
            for (int d = 0; d < 7; d++) {
                if (d % docCount == i % docCount ||
                    (d + 2) % docCount == i % docCount ||
                    (d + 4) % docCount == i % docCount) {
                    schedule[days[d]].append(doctors[i]);
                }
            }
        }
        for (const auto& day : days) {
            if (schedule[day].isEmpty())
                schedule[day].append(doctors[0]);
        }
    }

    auto* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(32,28,32,28);
    mainLay->setSpacing(20);

    auto* title = new QLabel("Doctors Schedule");
    title->setObjectName("pageTitle");
    mainLay->addWidget(title);

    auto* manageCard = new QFrame();
    manageCard->setObjectName("card");
    auto* manageLay = new QHBoxLayout(manageCard);
    manageLay->setContentsMargins(20,16,20,16);
    manageLay->setSpacing(12);

    auto* manageLbl = new QLabel("Manage Schedule");
    manageLbl->setObjectName("sectionTitle");
    manageLay->addWidget(manageLbl);
    manageLay->addStretch();

    auto* addBtn = new QPushButton("+ Add Doctor");
    addBtn->setObjectName("primaryBtn");
    addBtn->setFixedHeight(40);
    addBtn->setFixedWidth(140);
    manageLay->addWidget(addBtn);
    connect(addBtn, &QPushButton::clicked, this, &SchedulePage::showAddDoctorDialog);

    auto* removeBtn = new QPushButton("Remove Doctor");
    removeBtn->setObjectName("dangerBtn");
    removeBtn->setFixedHeight(40);
    removeBtn->setFixedWidth(150);
    manageLay->addWidget(removeBtn);
    connect(removeBtn, &QPushButton::clicked, this, &SchedulePage::showRemoveDoctorDialog);

    mainLay->addWidget(manageCard);

    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* gridContainer = new QWidget();
    auto* gridOuterLay  = new QHBoxLayout(gridContainer);
    gridOuterLay->setContentsMargins(0,0,0,0);
    gridOuterLay->setSpacing(12);

    auto* calendarCard = new QFrame();
    calendarCard->setObjectName("card");
    auto* calLay = new QHBoxLayout(calendarCard);
    calLay->setContentsMargins(16,16,16,16);
    calLay->setSpacing(8);

    for (const auto& day : days) {
        auto* col = new QVBoxLayout();
        col->setSpacing(8);

        auto* dayLbl = new QLabel(day);
        dayLbl->setStyleSheet("font-weight: bold; color: #0D1B2A; font-size: 13px;"
                              "border-bottom: 2px solid #00B4D8; padding-bottom: 6px;");
        col->addWidget(dayLbl);

        dayLayouts[day] = col;
        col->addStretch();
        calLay->addLayout(col, 1);
    }

    gridOuterLay->addWidget(calendarCard, 3);

    auto* availCard = new QFrame();
    availCard->setObjectName("card");
    availCard->setFixedWidth(200);
    auto* availLay = new QVBoxLayout(availCard);
    availLay->setContentsMargins(14,14,14,14);
    availLay->setSpacing(8);

    auto* availTitle = new QLabel("Available Doctors");
    availTitle->setStyleSheet("font-weight: bold; color: #0D1B2A; font-size: 13px;");
    availLay->addWidget(availTitle);

    availDoctorsList = new QListWidget();
    availDoctorsList->setFrameShape(QFrame::NoFrame);

    availDoctorsList->setStyleSheet(R"(
        QListWidget {
            border: none;
            background-color: #ffffff;
            color: #000000;
        }
        QListWidget::item {
            border: 1px solid #e5e7eb;
            border-radius: 6px;
            margin-bottom: 6px;
            padding: 8px;
            min-height: 44px;
            color: #000000;
        }
        QListWidget::item:selected {
            background: #e0f7fa;
            border-color: #00B4D8;
            color: #000000;
        }
        QListWidget::item:hover {
            background: #f0fdff;
            color: #000000;
        }
    )");

    for (int i = 0; i < docCount; i++) {
        auto* item = new QListWidgetItem(
            QString::fromStdString(doctors[i]->name) + "\n" +
            QString::fromStdString(doctors[i]->spec));
        item->setSizeHint(QSize(180, 56));
        availDoctorsList->addItem(item);
    }
    availDoctorsList->setUniformItemSizes(false);
    availDoctorsList->setSpacing(2);
    availLay->addWidget(availDoctorsList);
    gridOuterLay->addWidget(availCard);

    scroll->setWidget(gridContainer);
    mainLay->addWidget(scroll);

    refreshGrid();
}

QWidget* SchedulePage::makeDoctorCard(Staff::Doctor* doc) {
    auto* card = new QFrame();
    card->setStyleSheet("QFrame { background: #f8fafc; border: 1px solid #e5e7eb; "
                        "border-radius: 6px; padding: 4px; }");
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(8,8,8,8);
    cl->setSpacing(4);

    auto* nameLbl = new QLabel(QString::fromStdString(doc->name));
    nameLbl->setStyleSheet("font-weight: bold; font-size: 12px; color: #0D1B2A;");
    nameLbl->setWordWrap(true);
    cl->addWidget(nameLbl);

    auto* specLbl = new QLabel(QString::fromStdString(doc->spec));
    specLbl->setStyleSheet("font-size: 11px; color: #6b7280;");
    cl->addWidget(specLbl);

    QString spec = QString::fromStdString(doc->spec);
    QString badgeStyle;
    if      (spec == "Cardiology")   badgeStyle = "background:#fee2e2; color:#dc2626;";
    else if (spec == "ENT")          badgeStyle = "background:#dbeafe; color:#2563eb;";
    else if (spec == "Dermatology")  badgeStyle = "background:#f3e8ff; color:#7c3aed;";
    else                             badgeStyle = "background:#f0fdf4; color:#16a34a;";

    auto* badge = new QLabel(spec);
    badge->setStyleSheet("border-radius: 8px; padding: 2px 8px; font-size: 11px; font-weight: bold;" + badgeStyle);
    badge->setFixedHeight(20);
    cl->addWidget(badge);
    return card;
}

void SchedulePage::refreshGrid() {
    for (const auto& day : days) {
        auto* col = dayLayouts[day];
        while (col->count() > 2) {
            auto* item = col->takeAt(1);
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        auto* stretch = col->takeAt(col->count() - 1);
        delete stretch;

        if (schedule[day].isEmpty()) {
            auto* emptyLbl = new QLabel("No shifts");
            emptyLbl->setStyleSheet("color: #9ca3af; font-size: 12px; padding: 8px;");
            col->addWidget(emptyLbl);
        } else {
            for (auto* doc : schedule[day]) {
                col->addWidget(makeDoctorCard(doc));
            }
        }
        col->addStretch();
    }
}

void SchedulePage::showAddDoctorDialog() {
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Add Doctor to Schedule");
    dlg->setFixedWidth(460);
    dlg->setModal(true);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(14);

    auto* title = new QLabel("Add Doctor to Schedule");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#0D1B2A;");
    lay->addWidget(title);

    lay->addWidget(new QLabel("Doctor"));
    auto* docCombo = new QComboBox();
    docCombo->setFixedHeight(40);
    for (int i = 0; i < docCount; i++)
        docCombo->addItem(QString("[%1] %2 — %3")
                              .arg(doctors[i]->getId())
                              .arg(QString::fromStdString(doctors[i]->name))
                              .arg(QString::fromStdString(doctors[i]->spec)));
    lay->addWidget(docCombo);

    lay->addWidget(new QLabel("Assign to Days"));
    auto* daysGrid = new QGridLayout();
    daysGrid->setSpacing(8);
    QMap<QString, QCheckBox*> checkboxes;
    int col = 0, row = 0;
    for (const auto& day : days) {
        auto* cb = new QCheckBox(day);
        checkboxes[day] = cb;
        daysGrid->addWidget(cb, row, col);
        col++;
        if (col >= 2) { col = 0; row++; }
    }
    lay->addLayout(daysGrid);

    auto* btnRow = new QHBoxLayout();
    auto* addBtn    = new QPushButton("Add to Schedule");
    addBtn->setObjectName("primaryBtn");
    addBtn->setFixedHeight(40);
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("secondaryBtn");
    cancelBtn->setFixedHeight(40);
    btnRow->addWidget(addBtn);
    btnRow->addWidget(cancelBtn);
    lay->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(addBtn, &QPushButton::clicked, [&]() {
        int idx = docCombo->currentIndex();
        bool anyChecked = false;
        for (const auto& day : days) {
            if (checkboxes[day]->isChecked()) {
                anyChecked = true;
                bool found = false;
                for (auto* d : schedule[day])
                    if (d->getId() == doctors[idx]->getId()) { found = true; break; }
                if (!found) schedule[day].append(doctors[idx]);
            }
        }
        if (!anyChecked) {
            showStyledMessage(dlg, "No Days", "Please select at least one day.", QMessageBox::Warning);
            return;
        }
        refreshGrid();
        dlg->accept();
        showStyledMessage(this, "Done", QString::fromStdString(doctors[idx]->name) + " added to schedule.", QMessageBox::Information);
    });

    dlg->exec();
}

void SchedulePage::showRemoveDoctorDialog() {
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Remove Doctor from Schedule");
    dlg->setFixedWidth(420);
    dlg->setModal(true);

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(24,24,24,24);
    lay->setSpacing(14);

    auto* title = new QLabel("Remove Doctor from Schedule");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#0D1B2A;");
    lay->addWidget(title);

    auto* subLbl = new QLabel("Select a doctor and day to remove from the schedule:");
    subLbl->setStyleSheet("color:#6b7280;");
    subLbl->setWordWrap(true);
    lay->addWidget(subLbl);

    lay->addWidget(new QLabel("Doctor"));
    auto* docCombo = new QComboBox();
    docCombo->setFixedHeight(40);
    docCombo->addItem("Select doctor");
    for (int i = 0; i < docCount; i++)
        docCombo->addItem(QString::fromStdString(doctors[i]->name));
    lay->addWidget(docCombo);

    lay->addWidget(new QLabel("Day"));
    auto* dayCombo = new QComboBox();
    dayCombo->setFixedHeight(40);
    dayCombo->addItem("Select day");
    for (const auto& d : days) dayCombo->addItem(d);
    lay->addWidget(dayCombo);

    auto* warnLbl = new QLabel("⚠  This will remove the doctor from the selected day's schedule.");
    warnLbl->setWordWrap(true);
    warnLbl->setStyleSheet("background:#fffbeb; color:#92400e; border:1px solid #fde68a;"
                           "border-radius:6px; padding:10px;");
    lay->addWidget(warnLbl);

    auto* btnRow = new QHBoxLayout();
    auto* removeBtn = new QPushButton("Remove from Schedule");
    removeBtn->setObjectName("dangerBtn");
    removeBtn->setFixedHeight(40);
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("secondaryBtn");
    cancelBtn->setFixedHeight(40);
    btnRow->addWidget(removeBtn);
    btnRow->addWidget(cancelBtn);
    lay->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(removeBtn, &QPushButton::clicked, [&]() {
        int dIdx = docCombo->currentIndex();
        int dayIdx = dayCombo->currentIndex();
        if (dIdx <= 0 || dayIdx <= 0) {
            showStyledMessage(dlg, "Select", "Please select a doctor and a day.", QMessageBox::Warning);
            return;
        }
        Staff::Doctor* targetDoc = doctors[dIdx - 1];
        QString selectedDay = days[dayIdx - 1];
        auto& dayList = schedule[selectedDay];
        bool found = false;
        for (int i = 0; i < dayList.size(); i++) {
            if (dayList[i]->getId() == targetDoc->getId()) {
                dayList.removeAt(i);
                found = true; break;
            }
        }
        if (!found) {
            showStyledMessage(dlg, "Not Found", QString::fromStdString(targetDoc->name) + " is not scheduled on " + selectedDay + ".", QMessageBox::Warning);
            return;
        }
        refreshGrid();
        dlg->accept();
        showStyledMessage(this, "Done", QString::fromStdString(targetDoc->name) + " removed from " + selectedDay + ".", QMessageBox::Information);
    });

    dlg->exec();
}
