#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <QVector>

using namespace std;

namespace MedicalRecords {

class med_rec {
    string p_name;
    string p_id;
    int p_age;
    string recordID;
    string date;
    string clinicDepartment;

    string diagnosis;
    int doctorID;
public:
    med_rec(string name, string id, int age, string rID, string d, string clinic, string diag, int drID) {
        p_name = name;
        p_id = id;
        p_age = age;
        recordID = rID;
        date = d;
        clinicDepartment = clinic;
         diagnosis = diag;
        doctorID = drID;
    }
        void display_mr(unordered_map<int, string>& doc_db) {

        string doc_name = "Unknown Doctor";

        if (doc_db.find(doctorID) != doc_db.end()) {
            doc_name = doc_db[doctorID];
        }

        cout << "\n--- Medical Record: " << recordID << " ---" << endl;
        cout << "Patient: " << p_name << " (ID: " << p_id << ", Age: " << p_age << ")" << endl;
        cout << "Date: " << date << " | Clinic: " << clinicDepartment << endl;
        cout << "Doctor: " << doc_name << " (ID: " << doctorID << ") | Diagnosis: " << diagnosis << endl;
        cout << "---------------------------------" << endl;
    }

    string get_record_id() { return recordID; }
    string get_patient_id() { return p_id; }
    string get_date() { return date; }
    string get_clinic() { return clinicDepartment; }
    string get_diagnosis() { return diagnosis; }
   int get_doctor_id() { return doctorID; }
   void set_diagnosis(string new_diag) { diagnosis = new_diag; }
};

class patient {
    string p_name;
    string p_id;
    int p_age;
    bool has_insurance;
    string insurance_name;
    med_rec** history;
    int history_size;

public:
    patient(string name, string id, int age,bool has_ins, string ins_name) {
        p_name = name;
        p_id = id;
        p_age = age;
        has_insurance = has_ins;
        insurance_name = ins_name;
        history = nullptr;
        history_size = 0;  
    }
    void add_record(med_rec* new_record) {
        med_rec** new_history = new med_rec * [history_size + 1];
        for (int i = 0; i < history_size; i++) {
            new_history[i] = history[i];
        }
        new_history[history_size] = new_record;
        delete[] history;
        history = new_history;
        history_size++;
    }
    void display_history(unordered_map<int, string>& doc_db) {
        cout << "\n=================================================" << endl;
        cout << "  MEDICAL FILE: " << p_name << " (ID: " << p_id << ", Age: " << p_age << ")" << endl;

        if (has_insurance) { cout << "  BILLING: Insured via " << insurance_name << endl; }
        else { cout << "  BILLING: No Insurance " << endl; }
        cout << "=================================================" << endl;

        if (history_size == 0) {
            cout << "No past records found for this patient." << endl;
        }
        else {
            for (int i = 0; i < history_size; i++) {
                history[i]->display_mr(doc_db);
            }
        }
        cout << "=================================================\n" << endl;
    }


    bool has_duplicate_record(string check_id) {
        for (int i = 0; i < history_size; i++) {
            if (history[i]->get_record_id() == check_id) {
                return true;
            }
        }
        return false;
    }
    bool remove_record(string r_id) {
        int target_index = -1;
        for (int i = 0; i < history_size; i++) {
            if (history[i]->get_record_id() == r_id) {
                target_index = i;
                break;
            }
        }

        if (target_index == -1) { return false; }

        delete history[target_index];

        if (history_size == 1) {
            delete[] history;
            history = nullptr;
            history_size = 0;
            return true;
        }

        med_rec** new_history = new med_rec * [history_size - 1];

        int new_i = 0;
        for (int i = 0; i < history_size; i++) {
            if (i != target_index) {
                new_history[new_i] = history[i];
                new_i++;
            }
        }

        delete[] history;
        history = new_history;
        history_size--;

        return true;
    }

    med_rec* get_record_by_id(string r_id) {
        for (int i = 0; i < history_size; i++) {
            if (history[i]->get_record_id() == r_id) {
                return history[i];
            }
        }
        return nullptr;
    }

    int get_history_size() { return history_size; }
    med_rec* get_record_by_index(int index) { return history[index]; }
    string get_p_id() { return p_id; }
        string get_name() { return p_name; }
        int get_age() { return p_age; }
        string get_insurance() {
            if (has_insurance) {
                return insurance_name;
            }
            else {
                return "None";
            }
        }

    QVector<med_rec*> get_history() {
        QVector<med_rec*> result;
        for (int i = 0; i < history_size; i++)
            result.append(history[i]);
        return result;
    }

};


inline void save_patient_to_file(patient* p) {
    ofstream outFile("patient.txt", ios::app);
    if (outFile.is_open()) {
        outFile << p->get_p_id() << ","
            << p->get_name() << ","
            << p->get_age() << ","
            << p->get_insurance() << endl;
        outFile.close();
    }
}

inline void load_patients_from_file(QVector<patient*>& patients) {
    ifstream inFile("patient.txt");
    if (!inFile.is_open()) {
        cout << "No existing patient database found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, name, age_str, ins_name;
        int age;
        bool has_ins = false;
        getline(ss, id,      ',');
        getline(ss, name,    ',');
        getline(ss, age_str, ',');
        getline(ss, ins_name);
        age = stoi(age_str);
        if (ins_name != "None") { has_ins = true; }
        patient* loaded_p = new patient(name, id, age, has_ins, ins_name);
        patients.append(loaded_p);
    }
    inFile.close();
    cout << "Database Loaded: All previous patients restored to the system!\n";
}

inline void save_record_to_file(med_rec* mr) {
    ofstream outFile("mr.txt", ios::app);
    if (outFile.is_open()) {
        outFile << mr->get_record_id() << ","
            << mr->get_patient_id() << ","
            << mr->get_date() << ","
            << mr->get_clinic() << ","
            << mr->get_diagnosis() << ","
            << mr->get_doctor_id() << endl;
        outFile.close();
    }
}

inline void load_records_from_file(QVector<patient*>& patients) {
    ifstream inFile("mr.txt");
    if (!inFile.is_open()) {
        cout << "No existing medical records found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string r_id, p_id, date, clinic, diag, doc_id_str;
        int doc_id;
        getline(ss, r_id,       ',');
        getline(ss, p_id,       ',');
        getline(ss, date,       ',');
        getline(ss, clinic,     ',');
        getline(ss, diag,       ',');
        getline(ss, doc_id_str);
        doc_id = stoi(doc_id_str);
        for (auto* p : patients) {
            if (p->get_p_id() == p_id) {
                string p_name = p->get_name();
                int p_age = p->get_age();
                med_rec* loaded_mr = new med_rec(p_name, p_id, p_age, r_id, date, clinic, diag, doc_id);
                p->add_record(loaded_mr);
                break;
            }
        }
    }
    inFile.close();
    cout << "Database Loaded: All medical records restored to the system!\n";
}

inline void update_mr_file(const QVector<patient*>& patients) {
    ofstream outFile("mr.txt", ios::trunc);
    if (outFile.is_open()) {
        for (auto* p : patients) {
            for (int i = 0; i < p->get_history_size(); i++) {
                med_rec* mr = p->get_record_by_index(i);
                outFile << mr->get_record_id() << ","
                    << mr->get_patient_id() << ","
                    << mr->get_date() << ","
                    << mr->get_clinic() << ","
                    << mr->get_diagnosis() << ","
                    << mr->get_doctor_id() << endl;
            }
        }
        outFile.close();
    }
}

}


namespace Staff {

class Doctor {
    int id;
public:
    string name;
    string spec;

    Doctor(int id, string name, string spec) : id(id), name(name), spec(spec) {}
    int getId() const { return id; }
};

inline void load_doctors_from_file(Doctor**& docs, int& count) {
    ifstream inFile("doctors.txt");
    count = 0;
    if (!inFile.is_open()) return;

    string line;
    while (getline(inFile, line)) if (!line.empty()) count++;
    if (count == 0) return;

    inFile.clear();
    inFile.seekg(0);
    docs = new Doctor*[count];
    int i = 0;
    while (getline(inFile, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id_str, name, spec;
        getline(ss, id_str, ',');
        getline(ss, name,   ',');
        getline(ss, spec,   ',');
        docs[i++] = new Doctor(stoi(id_str), name, spec);
    }
    inFile.close();
    cout << "Doctor Database Loaded: Staff files successfully integrated!\n";
}

}
