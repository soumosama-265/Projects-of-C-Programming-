#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PATIENTS_FILE     "patients.dat"
#define DELETION_LOG_FILE "deletion_log.dat"
#define ID_TRACKER_FILE   "last_id.dat"

typedef struct {
    int  patient_id;
    char name[100];
    int  age;
    char gender[10];
    char contact[11];
    char disease[150];
    char doctor[100];
    int  room_number;
    char admission_date[11];
    char discharge_date[11];
    int  is_deleted;
} Patient;

typedef struct {
    int  log_id;
    int  patient_id;
    char patient_name[100];
    char reason[255];
    char deleted_at[25];
} DeletionLog;


int myStrLen(char *s) {
    int i = 0;
    while (s[i] != '\0') i++;
    return i;
}

void myStrCopy(char *dest, char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int myStrCmp(char *a, char *b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return a[i] - b[i];
        i++;
    }
    return a[i] - b[i];
}

void myMemSet(void *ptr, int value, int size) {
    char *p = (char *)ptr;
    for (int i = 0; i < size; i++) p[i] = (char)value;
}

void myStrNCopy(char *dest, char *src, int n) {
    int i = 0;
    while (i < n - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void readLine(char *buffer, int maxLen) {
    int i = 0;
    char c;
    while (i < maxLen - 1) {
        c = getchar();
        if (c == '\n' || c == EOF) break;
        buffer[i] = c;
        i++;
    }
    buffer[i] = '\0';
    if (i == maxLen - 1) {
        while ((c = getchar()) != '\n' && c != EOF);
    }
}

void pressEnterToContinue() {
    printf("\nPress Enter to continue...");
    clearInputBuffer();
}

int isValidGender(char *g) {
    return (myStrCmp(g, "Male") == 0 ||
            myStrCmp(g, "Female") == 0 ||
            myStrCmp(g, "Other") == 0);
}

int isValidDate(char *date) {
    if (myStrLen(date) != 10) return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') return 0;
        } else {
            if (!isDigit(date[i])) return 0;
        }
    }
    return 1;
}

int isValidContact(char *contact) {
    if (myStrLen(contact) != 10) return 0;
    for (int i = 0; i < 10; i++) {
        if (!isDigit(contact[i])) return 0;
    }
    return 1;
}

int getNextPatientID() {
    FILE *f = fopen(ID_TRACKER_FILE, "rb");
    int lastID = 0;
    if (f != NULL) {
        fread(&lastID, sizeof(int), 1, f);
        fclose(f);
    }
    lastID++;
    f = fopen(ID_TRACKER_FILE, "wb");
    if (f == NULL) {
        printf("Critical error: Cannot write to ID tracker file. Exiting.\n");
        exit(1);
    }
    fwrite(&lastID, sizeof(int), 1, f);
    fclose(f);
    return lastID;
}

int getNextLogID() {
    FILE *f = fopen(DELETION_LOG_FILE, "rb");
    int count = 0;
    if (f != NULL) {
        DeletionLog log;
        while (fread(&log, sizeof(DeletionLog), 1, f) == 1) count++;
        fclose(f);
    }
    return count + 1;
}

void getCurrentDateTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    int year  = tm_info->tm_year + 1900;
    int month = tm_info->tm_mon + 1;
    int day   = tm_info->tm_mday;
    int hour  = tm_info->tm_hour;
    int min   = tm_info->tm_min;
    int sec   = tm_info->tm_sec;
    buffer[0]  = '0' + year / 1000;
    buffer[1]  = '0' + (year / 100) % 10;
    buffer[2]  = '0' + (year / 10) % 10;
    buffer[3]  = '0' + year % 10;
    buffer[4]  = '-';
    buffer[5]  = '0' + month / 10;
    buffer[6]  = '0' + month % 10;
    buffer[7]  = '-';
    buffer[8]  = '0' + day / 10;
    buffer[9]  = '0' + day % 10;
    buffer[10] = ' ';
    buffer[11] = '0' + hour / 10;
    buffer[12] = '0' + hour % 10;
    buffer[13] = ':';
    buffer[14] = '0' + min / 10;
    buffer[15] = '0' + min % 10;
    buffer[16] = ':';
    buffer[17] = '0' + sec / 10;
    buffer[18] = '0' + sec % 10;
    buffer[19] = '\0';
}

int patientExists(int id, Patient *out) {
    FILE *f = fopen(PATIENTS_FILE, "rb");
    if (f == NULL) return 0;
    Patient p;
    while (fread(&p, sizeof(Patient), 1, f) == 1) {
        if (p.patient_id == id && p.is_deleted == 0) {
            if (out != NULL) *out = p;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int wasEverAdded(int id) {
    FILE *f = fopen(PATIENTS_FILE, "rb");
    if (f == NULL) return 0;
    Patient p;
    while (fread(&p, sizeof(Patient), 1, f) == 1) {
        if (p.patient_id == id) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void writePatientToFile(Patient p) {
    FILE *f = fopen(PATIENTS_FILE, "ab");
    if (f == NULL) {
        printf("Error: Cannot open patients file for writing.\n");
        printf("Reason: The file may be locked or you may not have write permissions.\n");
        return;
    }
    fwrite(&p, sizeof(Patient), 1, f);
    fclose(f);
}

void writeDeletionLog(DeletionLog log) {
    FILE *f = fopen(DELETION_LOG_FILE, "ab");
    if (f == NULL) {
        printf("Warning: Could not open deletion log file. This deletion was not logged.\n");
        return;
    }
    fwrite(&log, sizeof(DeletionLog), 1, f);
    fclose(f);
}


void addPatients() {
    int count;
    printf("\n--- ADD NEW PATIENT RECORDS ---\n");
    printf("How many patient records do you want to add? ");
    if (scanf("%d", &count) != 1 || count <= 0) {
        printf("Invalid input. Please enter a positive number.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    for (int i = 1; i <= count; i++) {
        printf("\nEntering details for Patient %d of %d:\n", i, count);

        Patient p;
        myMemSet(&p, 0, sizeof(Patient));
        p.is_deleted = 0;

        printf("  Full Name: ");
        readLine(p.name, sizeof(p.name));
        if (myStrLen(p.name) == 0) {
            printf("  Name cannot be empty. Skipping this record.\n");
            i--;
            continue;
        }

        printf("  Age: ");
        if (scanf("%d", &p.age) != 1 || p.age <= 0 || p.age > 120) {
            printf("  Invalid age. Must be between 1 and 120. Skipping this record.\n");
            clearInputBuffer();
            i--;
            continue;
        }
        clearInputBuffer();

        printf("  Gender (Male / Female / Other): ");
        readLine(p.gender, sizeof(p.gender));
        if (!isValidGender(p.gender)) {
            printf("  Invalid gender. Please type exactly: Male, Female, or Other. Skipping this record.\n");
            i--;
            continue;
        }

        printf("  Contact Number (10 digits, no spaces): ");
        readLine(p.contact, sizeof(p.contact));
        if (!isValidContact(p.contact)) {
            printf("  Invalid contact. Must be exactly 10 digits. Skipping this record.\n");
            i--;
            continue;
        }

        printf("  Disease / Diagnosis (press Enter to mark as 'Not diagnosed yet'): ");
        readLine(p.disease, sizeof(p.disease));
        if (myStrLen(p.disease) == 0) {
            myStrCopy(p.disease, "Not diagnosed yet");
            printf("  Disease set to: Not diagnosed yet\n");
        }

        printf("  Assigned Doctor Name: ");
        readLine(p.doctor, sizeof(p.doctor));
        if (myStrLen(p.doctor) == 0) {
            printf("  Doctor name cannot be empty. Skipping this record.\n");
            i--;
            continue;
        }

        printf("  Room Number: ");
        if (scanf("%d", &p.room_number) != 1 || p.room_number <= 0) {
            printf("  Invalid room number. Must be a positive number. Skipping this record.\n");
            clearInputBuffer();
            i--;
            continue;
        }
        clearInputBuffer();

        printf("  Admission Date (YYYY-MM-DD): ");
        readLine(p.admission_date, sizeof(p.admission_date));
        if (!isValidDate(p.admission_date)) {
            printf("  Invalid date format. Please use YYYY-MM-DD. Skipping this record.\n");
            i--;
            continue;
        }

        printf("  Discharge Date (YYYY-MM-DD, or press Enter if not yet discharged): ");
        char dischargeInput[11];
        readLine(dischargeInput, sizeof(dischargeInput));
        if (myStrLen(dischargeInput) == 0) {
            myStrCopy(p.discharge_date, "");
        } else {
            if (!isValidDate(dischargeInput)) {
                printf("  Invalid discharge date format. Use YYYY-MM-DD. Skipping this record.\n");
                i--;
                continue;
            }
            myStrCopy(p.discharge_date, dischargeInput);
        }

        p.patient_id = getNextPatientID();
        writePatientToFile(p);
        printf("  Record saved successfully. Patient ID assigned: %d\n", p.patient_id);
    }
}

void viewPatient() {
    printf("\n--- VIEW PATIENT RECORD ---\n");
    printf("Enter the Patient ID to view: ");
    int id;
    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid ID. Please enter a positive number.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    Patient p;
    if (patientExists(id, &p)) {
        printf("\n========================================\n");
        printf("  Patient ID       : %d\n",  p.patient_id);
        printf("  Name             : %s\n",  p.name);
        printf("  Age              : %d\n",  p.age);
        printf("  Gender           : %s\n",  p.gender);
        printf("  Contact          : %s\n",  p.contact);
        printf("  Disease          : %s\n",  p.disease);
        printf("  Doctor Assigned  : %s\n",  p.doctor);
        printf("  Room Number      : %d\n",  p.room_number);
        printf("  Admission Date   : %s\n",  p.admission_date);
        printf("  Discharge Date   : %s\n",
            myStrLen(p.discharge_date) == 0 ? "Not yet discharged" : p.discharge_date);
        printf("========================================\n");
    } else if (wasEverAdded(id)) {
        printf("No active record found for Patient ID %d.\n", id);
        printf("Reason: This patient record was deleted from the system.\n");
        printf("Check the deletion log (Option 6) to see when and why it was deleted.\n");
    } else {
        printf("No patient found with ID %d.\n", id);
        printf("Possible reasons:\n");
        printf("  - This ID was never registered in the system.\n");
        printf("  - You may have entered the wrong ID. Please check and try again.\n");
    }
}

void viewAllPatients() {
    printf("\n--- ALL PATIENT RECORDS ---\n");

    FILE *f = fopen(PATIENTS_FILE, "rb");
    if (f == NULL) {
        printf("No patient records file found.\n");
        printf("Reason: No patients have been added to the system yet.\n");
        return;
    }

    Patient p;
    int count = 0;
    Patient records[1000];

    while (fread(&p, sizeof(Patient), 1, f) == 1) {
        if (p.is_deleted == 0) {
            records[count] = p;
            count++;
        }
    }
    fclose(f);

    if (count == 0) {
        printf("No active patient records found.\n");
        printf("Reason: All records may have been deleted, or none have been added yet.\n");
        return;
    }

    printf("Total active records: %d\n\n", count);
    printf("%-5s %-20s %-4s %-7s %-22s %-20s %-5s %-12s\n",
           "ID", "Name", "Age", "Gender", "Disease", "Doctor", "Room", "Admitted");
    printf("--------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-4d %-7s %-22s %-20s %-5d %-12s\n",
            records[i].patient_id,
            records[i].name,
            records[i].age,
            records[i].gender,
            records[i].disease,
            records[i].doctor,
            records[i].room_number,
            records[i].admission_date);
    }
}

void updatePatient() {
    printf("\n--- UPDATE PATIENT RECORD ---\n");
    printf("Note: Only currently admitted patients (not yet discharged) can be updated.\n");
    printf("You can update: Disease, Doctor, Room Number, or set a Discharge Date.\n\n");

    printf("Enter the Patient ID to update: ");
    int id;
    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid ID. Please enter a positive number.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    Patient p;
    if (!patientExists(id, &p)) {
        if (wasEverAdded(id)) {
            printf("Cannot update Patient ID %d.\n", id);
            printf("Reason: This record was deleted. Deleted records cannot be modified.\n");
        } else {
            printf("No patient found with ID %d.\n", id);
            printf("Reason: This ID has never been registered. Please verify the ID and try again.\n");
        }
        return;
    }

    if (myStrLen(p.discharge_date) > 0) {
        printf("Cannot update record for '%s' (ID: %d).\n", p.name, id);
        printf("Reason: This patient was discharged on %s.\n", p.discharge_date);
        printf("Discharged patient records are locked to protect data integrity.\n");
        return;
    }

    printf("Updating record for: %s (ID: %d)\n", p.name, id);
    printf("What do you want to update?\n");
    printf("  1. Disease / Diagnosis\n");
    printf("  2. Doctor Assigned\n");
    printf("  3. Room Number\n");
    printf("  4. Set Discharge Date\n");
    printf("Your choice: ");

    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input. Update cancelled.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (choice == 1) {
        char disease[150];
        printf("Enter new Disease / Diagnosis: ");
        readLine(disease, sizeof(disease));
        if (myStrLen(disease) == 0) {
            printf("Disease field cannot be empty. Update cancelled.\n");
            return;
        }
        myStrCopy(p.disease, disease);

    } else if (choice == 2) {
        char doctor[100];
        printf("Enter new Doctor Name: ");
        readLine(doctor, sizeof(doctor));
        if (myStrLen(doctor) == 0) {
            printf("Doctor name cannot be empty. Update cancelled.\n");
            return;
        }
        myStrCopy(p.doctor, doctor);

    } else if (choice == 3) {
        int room;
        printf("Enter new Room Number: ");
        if (scanf("%d", &room) != 1 || room <= 0) {
            printf("Invalid room number. Update cancelled.\n");
            clearInputBuffer();
            return;
        }
        clearInputBuffer();
        p.room_number = room;

    } else if (choice == 4) {
        char discharge[11];
        printf("Enter Discharge Date (YYYY-MM-DD): ");
        readLine(discharge, sizeof(discharge));
        if (!isValidDate(discharge)) {
            printf("Invalid date format. Please use YYYY-MM-DD. Update cancelled.\n");
            return;
        }
        myStrCopy(p.discharge_date, discharge);

    } else {
        printf("Invalid option. Update cancelled.\n");
        return;
    }

    FILE *f = fopen(PATIENTS_FILE, "rb+");
    if (f == NULL) {
        printf("Error: Cannot open patients file for updating.\n");
        printf("Reason: The file may be missing or you may not have write permissions.\n");
        return;
    }

    Patient temp;
    while (fread(&temp, sizeof(Patient), 1, f) == 1) {
        if (temp.patient_id == id && temp.is_deleted == 0) {
            fseek(f, -(long)sizeof(Patient), SEEK_CUR);
            fwrite(&p, sizeof(Patient), 1, f);
            fclose(f);
            printf("Record for Patient ID %d updated successfully.\n", id);
            return;
        }
    }

    fclose(f);
    printf("Update failed. Record could not be located inside the file.\n");
}

void deletePatient() {
    printf("\n--- DELETE PATIENT RECORD ---\n");
    printf("A record can be deleted only under these conditions:\n");
    printf("  1. The patient has been discharged.\n");
    printf("  2. The record is a duplicate entry.\n");
    printf("  3. The record was entered by mistake.\n\n");

    printf("Enter the Patient ID to delete: ");
    int id;
    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid ID. Please enter a positive number.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    Patient p;
    if (!patientExists(id, &p)) {
        if (wasEverAdded(id)) {
            printf("Cannot delete Patient ID %d.\n", id);
            printf("Reason: This record was already deleted. Check the deletion log (Option 6) for details.\n");
        } else {
            printf("No patient found with ID %d.\n", id);
            printf("Reason: This ID was never registered in the system.\n");
        }
        return;
    }

    printf("Select the reason for deletion:\n");
    printf("  1. Patient has been discharged and record is no longer needed.\n");
    printf("  2. Duplicate entry — this patient was registered more than once.\n");
    printf("  3. Erroneous entry — the record was added by mistake.\n");
    printf("Your choice (1/2/3): ");

    int reasonChoice;
    if (scanf("%d", &reasonChoice) != 1 || reasonChoice < 1 || reasonChoice > 3) {
        printf("Invalid selection. Deletion cancelled.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (reasonChoice == 1 && myStrLen(p.discharge_date) == 0) {
        printf("Cannot delete record for '%s' (ID: %d).\n", p.name, id);
        printf("Reason: This patient has not been discharged yet.\n");
        printf("Please set a discharge date first (use Option 4 then choose 4), then delete.\n");
        return;
    }

    char reason[255];
    if (reasonChoice == 1) {
        myStrCopy(reason, "Patient discharged - record removed from active system.");
    } else if (reasonChoice == 2) {
        myStrCopy(reason, "Duplicate entry - patient was registered more than once.");
    } else {
        myStrCopy(reason, "Erroneous entry - record was added by mistake with incorrect data.");
    }

    printf("Are you sure you want to delete the record for '%s' (ID: %d)? (yes / no): ", p.name, id);
    char confirm[5];
    readLine(confirm, sizeof(confirm));

    if (myStrCmp(confirm, "yes") != 0) {
        printf("Deletion cancelled.\n");
        return;
    }

    FILE *f = fopen(PATIENTS_FILE, "rb+");
    if (f == NULL) {
        printf("Error: Cannot open patients file for deletion.\n");
        printf("Reason: The file may be missing or permissions are restricted.\n");
        return;
    }

    Patient temp;
    while (fread(&temp, sizeof(Patient), 1, f) == 1) {
        if (temp.patient_id == id && temp.is_deleted == 0) {
            temp.is_deleted = 1;
            fseek(f, -(long)sizeof(Patient), SEEK_CUR);
            fwrite(&temp, sizeof(Patient), 1, f);
            fclose(f);

            DeletionLog log;
            myMemSet(&log, 0, sizeof(DeletionLog));
            log.log_id     = getNextLogID();
            log.patient_id = id;
            myStrNCopy(log.patient_name, p.name, sizeof(log.patient_name));
            myStrNCopy(log.reason, reason, sizeof(log.reason));
            getCurrentDateTime(log.deleted_at);
            writeDeletionLog(log);

            printf("Record for '%s' (ID: %d) has been deleted.\n", p.name, id);
            printf("Reason saved: %s\n", reason);
            printf("This deletion has been recorded in the deletion log.\n");
            return;
        }
    }

    fclose(f);
    printf("Deletion failed. Record could not be located inside the file.\n");
}

void viewDeletionLog() {
    printf("\n--- DELETION LOG ---\n");

    FILE *f = fopen(DELETION_LOG_FILE, "rb");
    if (f == NULL) {
        printf("No deletion log found.\n");
        printf("Reason: No patient records have been deleted yet.\n");
        return;
    }

    DeletionLog log;
    int count = 0;
    DeletionLog logs[1000];

    while (fread(&log, sizeof(DeletionLog), 1, f) == 1) {
        logs[count] = log;
        count++;
    }
    fclose(f);

    if (count == 0) {
        printf("The deletion log is empty. No records have been deleted so far.\n");
        return;
    }

    printf("Total deletions logged: %d\n\n", count);
    printf("%-6s %-10s %-20s %-55s %-20s\n",
           "LogID", "PatientID", "Patient Name", "Reason", "Deleted At");
    printf("------------------------------------------------------------------------------------------------------------------\n");

    for (int i = count - 1; i >= 0; i--) {
        printf("%-6d %-10d %-20s %-55s %-20s\n",
            logs[i].log_id,
            logs[i].patient_id,
            logs[i].patient_name,
            logs[i].reason,
            logs[i].deleted_at);
    }
}

void displayMenu() {
    printf("\n========================================\n");
    printf("      HOSPITAL MANAGEMENT SYSTEM        \n");
    printf("========================================\n");
    printf("  1. Add New Patient Record(s)\n");
    printf("  2. View a Patient Record (by ID)\n");
    printf("  3. View All Patient Records\n");
    printf("  4. Update a Patient Record\n");
    printf("  5. Delete a Patient Record\n");
    printf("  6. View Deletion Log\n");
    printf("  7. Exit\n");
    printf("========================================\n");
    printf("Enter your choice: ");
}

int main() {
    printf("========================================\n");
    printf("   Welcome to Hospital Management System\n");
    printf("   (File Handling Version)              \n");
    printf("========================================\n");

    int choice;
    while (1) {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number between 1 and 7.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: addPatients();      pressEnterToContinue(); break;
            case 2: viewPatient();      pressEnterToContinue(); break;
            case 3: viewAllPatients();  pressEnterToContinue(); break;
            case 4: updatePatient();    pressEnterToContinue(); break;
            case 5: deletePatient();    pressEnterToContinue(); break;
            case 6: viewDeletionLog();  pressEnterToContinue(); break;
            case 7:
                printf("\nThank you for using the Hospital Management System.\n");
                printf("All records have been safely saved to the data files.\n");
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Please select a number between 1 and 7.\n");
        }
    }

    return 0;
}