#include "mainwindow.h"
#include "dashboardpage.h"
#include "appointmentspage.h"
#include "patientspage.h"
#include "financialpage.h"
#include "schedulepage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Al-Noor Hospital System");
    setMinimumSize(1280, 800);

    hospitalDoctors = Staff::load_doctors("doctors.txt", doctorCount);

    if (!hospitalDoctors || doctorCount == 0) {
        doctorCount = 0;
        hospitalDoctors = nullptr;

        auto add = [&](string n, string s, int id) {
            Staff::add_doctor(&hospitalDoctors, doctorCount,
                              new Staff::Doctor(n, s, id));
        };
        add("Dr. Samy Abdullah",  "ENT",         1001);
        add("Dr. Nagwa Alaa",     "ENT",         1002);
        add("Dr. Nour Hesham",    "ENT",         1003);
        add("Dr. Nelly Maged",    "Dermatology", 1004);
        add("Dr. Natallie Mina",  "Dermatology", 1005);
        add("Dr. Louay Salah",    "Dermatology", 1006);
        add("Dr. Sherif Mahmoud", "Cardiology",  1007);
        add("Dr. Hossam Yasser",  "Cardiology",  1008);
    }

    apptQueue = new Appointments::AppointmentQueue();

    auto loaded = MedicalRecords::load_patients("patient.txt");
    for (auto* p : loaded) patients.append(p);

    {
        vector<MedicalRecords::patient*> pVec;
        for (auto* p : patients) pVec.push_back(p);
        MedicalRecords::load_medical_records("mr.txt", pVec);
    }

    if (patients.isEmpty()) {
        auto* p1 = new MedicalRecords::patient("Basma","P-101",20,true,"AXA");
        auto* p2 = new MedicalRecords::patient("Adam", "P-102",19,true,"AXA");
        auto* p3 = new MedicalRecords::patient("Mai",  "P-103",19,true,"Allianz");
        auto* p4 = new MedicalRecords::patient("Zeyad waleed","P-104",21,true,"Allianz");
        patients.append(p1);
        patients.append(p2);
        patients.append(p3);
        patients.append(p4);
    }

    setupStyle();

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0,0,0,0);
    rootLayout->setSpacing(0);

    sidebar = new QWidget();
    sidebar->setFixedWidth(220);
    sidebar->setObjectName("sidebar");
    buildSidebar();
    rootLayout->addWidget(sidebar);

    billingManager = new Financials::BillingManager();

    stack = new QStackedWidget();
    stack->setObjectName("mainStack");

    dashPage  = new DashboardPage(apptQueue, &patients, &doctorCount, billingManager);
    apptPage  = new AppointmentsPage(hospitalDoctors, doctorCount, apptQueue);
    patPage   = new PatientsPage(patients, hospitalDoctors, doctorCount);
    finPage   = new FinancialPage();
    schedPage = new SchedulePage(hospitalDoctors, doctorCount);

    stack->addWidget(dashPage);
    stack->addWidget(apptPage);
    stack->addWidget(patPage);
    stack->addWidget(finPage);
    stack->addWidget(schedPage);

    connect(dashPage, &DashboardPage::requestBookAppointment, this, [this]{ switchPage(1); });
    connect(dashPage, &DashboardPage::requestNewBill,         this, [this]{ switchPage(3); });

    connect(apptPage, &AppointmentsPage::appointmentBooked, dashPage, &DashboardPage::refresh);

    connect(stack, &QStackedWidget::currentChanged, this, [this](int idx){
        if (idx == 0) dashPage->refresh();
    });

    rootLayout->addWidget(stack);

    switchPage(0);
}

MainWindow::~MainWindow() {
    delete apptQueue;
    delete billingManager;

    for (auto* p : patients) delete p;

    for (int i = 0; i < doctorCount; i++) delete hospitalDoctors[i];
    delete[] hospitalDoctors;
}

void MainWindow::buildSidebar() {
    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    auto* logo = new QLabel("Al-Noor\nHospital System");
    logo->setObjectName("sidebarLogo");
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    logo->setContentsMargins(20,24,20,24);
    layout->addWidget(logo);

    QVector<QPair<QString,int>> navItems = {
        {"  Dashboard",        0},
        {"  Appointments",     1},
        {"  Patients",         2},
        {"  Financial",        3},
        {"  Doctors Schedule", 4},
    };

    for (auto& item : navItems) {
        auto* btn = new QPushButton(item.first);
        btn->setObjectName("navBtn");
        btn->setCheckable(true);
        btn->setFixedHeight(44);
        btn->setFlat(true);
        int idx = item.second;
        connect(btn, &QPushButton::clicked, this, [this, idx]{ switchPage(idx); });
        navButtons.append(btn);
        layout->addWidget(btn);
    }

    layout->addStretch();
}

void MainWindow::switchPage(int index) {
    stack->setCurrentIndex(index);

    for (int i = 0; i < navButtons.size(); i++)
        navButtons[i]->setChecked(i == index);
}

void MainWindow::setupStyle() {
    qApp->setStyleSheet(R"(
/* ---- Global ---- */
QWidget { font-family: 'Segoe UI', Arial, sans-serif; font-size: 14px; }
QMainWindow { background: #f0f2f5; }

/* ---- Sidebar ---- */
QWidget#sidebar { background: #0D1B2A; }

QLabel#sidebarLogo {
    color: #ffffff;
    font-size: 16px;
    font-weight: bold;
    border-bottom: 1px solid #1e3a5f;
}

QPushButton#navBtn {
    color: #8ba3bc;
    background: transparent;
    border: none;
    text-align: left;
    padding-left: 20px;
    font-size: 14px;
}
QPushButton#navBtn:hover  { background: #1e3a5f; color: #ffffff; }
QPushButton#navBtn:checked { background: #1e3a5f; color: #00B4D8;
                              border-left: 3px solid #00B4D8; }

/* ---- Cards ---- */
QFrame#card {
    background: #ffffff;
    border-radius: 10px;
    border: 1px solid #e5e7eb;
}

/* ---- Section titles ---- */
QLabel#pageTitle {
    font-size: 22px;
    font-weight: bold;
    color: #0D1B2A;
}
QLabel#sectionTitle {
    font-size: 16px;
    font-weight: bold;
    color: #0D1B2A;
}
QLabel#kpiValue {
    font-size: 28px;
    font-weight: bold;
    color: #0D1B2A;
}
QLabel#kpiLabel { font-size: 13px; color: #6b7280; }
QLabel#kpiIcon  { font-size: 22px; }

/* ---- Primary button (teal) ---- */
QPushButton#primaryBtn {
    background: #00B4D8;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    padding: 8px 20px;
    font-weight: bold;
    font-size: 14px;
}
QPushButton#primaryBtn:hover { background: #0096b7; }

/* ---- Secondary button ---- */
QPushButton#secondaryBtn {
    background: #ffffff;
    color: #374151;
    border: 1px solid #d1d5db;
    border-radius: 8px;
    padding: 8px 20px;
    font-size: 14px;
}
QPushButton#secondaryBtn:hover { background: #f9fafb; }

/* ---- Danger button ---- */
QPushButton#dangerBtn {
    background: #E63946;
    color: #ffffff;
    border: none;
    border-radius: 8px;
    padding: 8px 20px;
    font-weight: bold;
    font-size: 14px;
}
QPushButton#dangerBtn:hover { background: #c0303c; }

/* ---- Table ---- */
QTableWidget {
    background: #ffffff;
    border: none;
    gridline-color: #f3f4f6;
    selection-background-color: #e0f7fa;
}
QTableWidget::item { padding: 8px; color: #374151; }
QHeaderView::section {
    background: #f9fafb;
    color: #6b7280;
    font-size: 12px;
    font-weight: bold;
    text-transform: uppercase;
    padding: 10px 8px;
    border: none;
    border-bottom: 1px solid #e5e7eb;
}

/* ---- Inputs ---- */
QLineEdit, QComboBox, QDateEdit, QTimeEdit {
    background: #f9fafb;
    border: 1px solid #e5e7eb;
    border-radius: 6px;
    padding: 8px 12px;
    color: #374151;
    font-size: 14px;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTimeEdit:focus {
    border: 1px solid #00B4D8;
    background: #ffffff;
}
QComboBox::drop-down { border: none; }
QComboBox::down-arrow { image: none; width: 0; }

/* ---- Scrollbar ---- */
QScrollBar:vertical { width: 6px; background: #f0f2f5; }
QScrollBar::handle:vertical { background: #d1d5db; border-radius: 3px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { height: 6px; background: #f0f2f5; }
QScrollBar::handle:horizontal { background: #d1d5db; border-radius: 3px; }

/* ---- List widget ---- */
QListWidget {
    background: #ffffff;
    border: 1px solid #e5e7eb;
    border-radius: 8px;
}
QListWidget::item { padding: 10px 14px; border-bottom: 1px solid #f3f4f6; }
QListWidget::item:selected { background: #e0f7fa; color: #0D1B2A; }
QListWidget::item:hover    { background: #f9fafb; }

/* ---- Dialog ---- */
QDialog {
    background: #ffffff;
    border-radius: 12px;
}

/* ---- CheckBox ---- */
QCheckBox { color: #374151; spacing: 8px; }
QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #d1d5db;
                        border-radius: 3px; background: #ffffff; }
QCheckBox::indicator:checked { background: #00B4D8; border-color: #00B4D8; }

/* ---- TextEdit ---- */
QTextEdit {
    background: #f9fafb;
    border: 1px solid #e5e7eb;
    border-radius: 6px;
    padding: 8px;
    color: #374151;
}
)");
}
