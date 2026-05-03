#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

class Doctor {
private:
    int id;
public:
    string name;
    string spec;
    Doctor(string n, string s, int id) : name(n), spec(s), id(id) {}
    int getId()
    {
        return id;
    }
};

class Clinic {
public:
    string name;
    string room;

    Clinic(string n, string r) : name(n), room(r) {}
};


void add_doctor(Doctor*** shift, int& size, Doctor* doc) {
    Doctor** new_shift = new Doctor * [size + 1];
    for (int i = 0; i < size; i++) {
        new_shift[i] = (*shift)[i];
    }
    new_shift[size] = doc;
    delete[] * shift;
    *shift = new_shift;
    size++;
}

void remove_doctor(Doctor*** shift, int& size) {
    if (size <= 0) return;
    Doctor** new_shift;

    if (size - 1 > 0) {
        new_shift = new Doctor * [size - 1];

        for (int i = 0; i < size - 1; i++) {
            new_shift[i] = (*shift)[i];
        }
    }
    else {
        new_shift = nullptr;
    }

    delete[] * shift;
    *shift = new_shift;
    size--;
}

void add_clinic(Clinic*** shift, int& size, Clinic* cln) {
    Clinic** new_shift = new Clinic * [size + 1];
    for (int i = 0; i < size; i++) {
        new_shift[i] = (*shift)[i];
    }
    new_shift[size] = cln;
    delete[] * shift;
    *shift = new_shift;
    size++;
}

void remove_clinic(Clinic*** shift, int& size) {
    if (size <= 0) return;
    Clinic** new_shift;

    if (size - 1 > 0) {
        new_shift = new Clinic * [size - 1];

        for (int i = 0; i < size - 1; i++) {
            new_shift[i] = (*shift)[i];
        }

    }
    else {
        new_shift = nullptr;
    }

    delete[] * shift;
    *shift = new_shift;
    size--;
}

Doctor** load_doctors(const string& filename, int& size) {
    ifstream file(filename);
    if (!file) {
        cout << "Error: could not open " << filename << endl;
        return nullptr;
    }
    Doctor** list = nullptr;
    size = 0;
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string Id, name, spec;
        getline(ss, Id, ',');
        getline(ss, name, ',');
        getline(ss, spec, ',');
        int id = stoi(Id);
        Doctor* doc = new Doctor(name, spec, id);
        add_doctor(&list, size, doc);
    }
    return list;
}


Clinic** load_clinics(const string& filename, int& size) {
    ifstream file(filename);
    if (!file) {
        cout << "Error: could not open " << filename << endl;
        return nullptr;
    }
    Clinic** list = nullptr;
    size = 0;
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string name, room;
        getline(ss, name, ',');
        getline(ss, room, ',');
        Clinic* cln = new Clinic(name, room);
        add_clinic(&list, size, cln);
    }
    return list;
}

void print_report(string day, Doctor** d_shift, int d_size, Clinic** c_shift, int c_size) {
    cout << "--- " << day << " Schedule ---" << endl;
    cout << "Doctors on Duty (" << d_size << "):" << endl;
    if (d_size == 0) cout << "  None" << endl;
    for (int i = 0; i < d_size; i++) {
        cout << "  - [" << d_shift[i]->getId() << "] " << d_shift[i]->name << " (" << d_shift[i]->spec << ")" << endl;
    }
    cout << "Open Clinics (" << c_size << "):" << endl;
    if (c_size == 0) cout << "  None" << endl;
    for (int i = 0; i < c_size; i++) {
        cout << "  - " << c_shift[i]->name << " [" << c_shift[i]->room << "]" << endl;
    }
    cout << "-----------------------------------" << endl << endl;
}