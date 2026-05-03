#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iostream>
using namespace std;

namespace Staff {

class Doctor {
private:
    int id;
public:
    string name;
    string spec;
    Doctor(string n, string s, int i) : name(n), spec(s), id(i) {}
    int getId() { return id; }
    string get_name() { return name; }
};

class Clinic {
public:
    string name;
    string room;
    Clinic(string n, string r) : name(n), room(r) {}
};

inline void add_doctor(Doctor*** shift, int& size, Doctor* doc) {
    Doctor** new_shift = new Doctor*[size + 1];
    for (int i = 0; i < size; i++) new_shift[i] = (*shift)[i];
    new_shift[size] = doc;
    delete[] *shift;
    *shift = new_shift;
    size++;
}

inline void remove_doctor(Doctor*** shift, int& size) {
    if (size <= 0) return;
    Doctor** new_shift = nullptr;
    if (size - 1 > 0) {
        new_shift = new Doctor*[size - 1];
        for (int i = 0; i < size - 1; i++) new_shift[i] = (*shift)[i];
    }
    delete[] *shift;
    *shift = new_shift;
    size--;
}

inline Doctor** load_doctors(const string& filename, int& size) {
    ifstream file(filename);
    if (!file) return nullptr;
    Doctor** list = nullptr;
    size = 0;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        stringstream ss(line);
        string Id, name, spec;
        getline(ss, Id, ',');
        getline(ss, name, ',');
        getline(ss, spec, ',');
        auto trim = [](string& s){
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t\r");
            s = (a == string::npos) ? "" : s.substr(a, b-a+1);
        };
        trim(Id); trim(name); trim(spec);
        if (Id.empty()) continue;
        int id = stoi(Id);
        Doctor* doc = new Doctor(name, spec, id);
        add_doctor(&list, size, doc);
    }
    return list;
}

}

namespace MedicalRecords {

class med_rec {
    string p_name, p_id, recordID, date, clinicDepartment, diagnosis;
    int p_age, doctorID;
public:
    med_rec(string name, string id, int age, string rID, string d,
            string clinic, string diag, int drID)
        : p_name(name), p_id(id), p_age(age), recordID(rID),
          date(d), clinicDepartment(clinic), diagnosis(diag), doctorID(drID) {}

    string get_record_id()  { return recordID; }
    string get_patient_id() { return p_id; }
    string get_date()       { return date; }
    string get_clinic()     { return clinicDepartment; }
    string get_diagnosis()  { return diagnosis; }
    int    get_doctor_id()  { return doctorID; }
    void   set_diagnosis(string d) { diagnosis = d; }
};

class patient {
    string p_name, p_id, insurance_name;
    int p_age;
    bool has_insurance;
    vector<med_rec*> history;
public:
    patient(string name, string id, int age, bool has_ins, string ins_name)
        : p_name(name), p_id(id), p_age(age),
          has_insurance(has_ins), insurance_name(ins_name) {}

    ~patient() { for (auto r : history) delete r; }

    void add_record(med_rec* r) { history.push_back(r); }

    string get_p_id()      { return p_id; }
    string get_name()      { return p_name; }
    int    get_age()       { return p_age; }
    string get_insurance() { return has_insurance ? insurance_name : "None"; }
    vector<med_rec*>& get_history() { return history; }
};

inline vector<patient*> load_patients(const string& filename) {
    vector<patient*> list;
    ifstream file(filename);
    if (!file) return list;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        stringstream ss(line);
        string id, name, ageStr, ins;
        getline(ss, id,     ',');
        getline(ss, name,   ',');
        getline(ss, ageStr, ',');
        getline(ss, ins,    ',');
        auto trim = [](string& s){
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            s = (a == string::npos) ? "" : s.substr(a, b-a+1);
        };
        trim(id); trim(name); trim(ageStr); trim(ins);
        if (id.empty() || name.empty()) continue;
        int age = ageStr.empty() ? 0 : stoi(ageStr);
        bool hasIns = (!ins.empty() && ins != "None");
        list.push_back(new patient(name, id, age, hasIns, ins));
    }
    return list;
}

inline void load_medical_records(const string& filename, vector<patient*>& patients) {
    ifstream file(filename);
    if (!file) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        stringstream ss(line);
        string recID, patID, date, clinic, diag, drIDStr;
        getline(ss, recID,   ',');
        getline(ss, patID,   ',');
        getline(ss, date,    ',');
        getline(ss, clinic,  ',');
        getline(ss, diag,    ',');
        getline(ss, drIDStr, ',');
        auto trim = [](string& s){
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            s = (a == string::npos) ? "" : s.substr(a, b-a+1);
        };
        trim(recID); trim(patID); trim(date); trim(clinic); trim(diag); trim(drIDStr);
        if (recID.empty() || patID.empty()) continue;
        int drID = drIDStr.empty() ? 0 : stoi(drIDStr);
        for (auto* p : patients) {
            if (p->get_p_id() == patID) {
                auto* rec = new med_rec(p->get_name(), patID, p->get_age(),
                                        recID, date, clinic, diag, drID);
                p->add_record(rec);
                break;
            }
        }
    }
}

}

namespace Financials {

struct Financialdata {
    int billId = 0;
    int PatientId = 0;
    string serviceDetails = "";
    double totalAmount = 0.0;
    double insuranceCoverage = 0.0;
    time_t issuedate = 0;
};

class Insurance {
    double AXA = 0.80, Metlife = 0.70, Allianz = 0.90;
public:
    double getCoverage(const string& provider) {
        if (provider == "AXA")     return AXA;
        if (provider == "Metlife") return Metlife;
        if (provider == "Allianz") return Allianz;
        return 0.0;
    }
};

class ClinicPricing {
public:
    double getBasePrice(const string& clinicName) {
        if (clinicName == "ENT")         return 500.00;
        if (clinicName == "Dermatology") return 600.00;
        if (clinicName == "Cardiology")  return 1000.00;
        return 0.0;
    }
};

class PaymentProcess {
    string cardNumber, cardHolderName, expiryDate, cvv;
    string hideCardNumber(const string& c) {
        if (c.length() < 6) return "Invalid Card";
        string m(c.length() - 4, '*');
        return m + c.substr(c.length() - 4);
    }
public:
    void setCardDetails(const string& num, const string& holder,
                        const string& exp, const string& cvvCode) {
        cardNumber = num; cardHolderName = holder;
        expiryDate = exp; cvv = cvvCode;
    }
    bool processTransaction(double amount) {
        if (cardNumber.length() < 8 || cvv.length() < 3) return false;
        cardNumber.clear(); cvv.clear();
        return true;
    }
    string getMaskedCard() {
        return cardNumber.empty() ? "" : hideCardNumber(cardNumber);
    }
};

class BillingManager {
    const string fileName = "financial_records.txt";
public:
    bool saveBill(const Financialdata& bill) {
        ofstream file(fileName, ios::app);
        if (!file.is_open()) return false;
        file << bill.billId << "|" << bill.PatientId << "|"
             << bill.serviceDetails << "|" << bill.totalAmount << "|"
             << bill.insuranceCoverage << "|" << bill.issuedate << "\n";
        file.close();
        return true;
    }
    vector<Financialdata> loadAllBills() {
        vector<Financialdata> all;
        ifstream file(fileName);
        if (!file.is_open()) return all;
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string t;
            Financialdata b;
            getline(ss, t, '|'); if (t.empty()) continue; b.billId = stoi(t);
            getline(ss, t, '|'); b.PatientId = stoi(t);
            getline(ss, b.serviceDetails, '|');
            getline(ss, t, '|'); b.totalAmount = stod(t);
            getline(ss, t, '|'); b.insuranceCoverage = stod(t);
            getline(ss, t, '|'); b.issuedate = stoll(t);
            all.push_back(b);
        }
        file.close();
        return all;
    }
};

}

namespace Appointments {

struct Appointment {
    string patient_name, doctor_name, date, time, clinic_name, status;
    Appointment(string p, string d, string dt, string t, string c, string s = "Scheduled")
        : patient_name(p), doctor_name(d), date(dt), time(t), clinic_name(c), status(s) {}
};

class AppointmentQueue {
    vector<Appointment> appointments;
public:
    bool isDoctorBusy(const string& docName, const string& time) {
        for (auto& a : appointments)
            if (a.doctor_name == docName && a.time == time) return true;
        return false;
    }
    bool book(const string& p, const string& doc, const string& d,
              const string& t, const string& c) {
        if (isDoctorBusy(doc, t)) return false;
        appointments.push_back(Appointment(p, doc, d, t, c));
        return true;
    }
    bool cancel(const string& p, const string& doc, const string& d, const string& t) {
        for (auto it = appointments.begin(); it != appointments.end(); ++it)
            if (it->patient_name == p && it->doctor_name == doc &&
                it->date == d && it->time == t) {
                it->status = "Cancelled";
                return true;
            }
        return false;
    }
    vector<Appointment>& getAll() { return appointments; }
    vector<Appointment> getByDate(const string& date) {
        vector<Appointment> result;
        for (auto& a : appointments)
            if (a.date == date) result.push_back(a);
        return result;
    }
};

}
