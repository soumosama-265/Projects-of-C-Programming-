# Projects-of-C-Programming-
1. Hospital Management System
# Hospital Management System - File Handling Version

## No database required. All data is stored in binary .dat files automatically.

---

## Data Files Created Automatically

When you run the program for the first time, three files are created in the same folder:

patients.dat
    Stores all patient records as binary structs.
    Records are never physically removed — deleted records are marked with a flag (is_deleted = 1)
    so the ID history is always preserved.

deletion_log.dat
    Stores a log of every deletion that has occurred, including the reason and timestamp.

last_id.dat
    Tracks the last auto-generated patient ID so IDs always increase and never repeat,
    even if records are deleted.

Do not manually edit or delete these files or data corruption will occur.

---

## How the File Handling Works

Each patient is stored as a fixed-size binary struct written using fwrite().
When reading, fread() scans record by record and filters by is_deleted flag.
When updating, the file is opened in rb+ mode, the matching record is located,
the file pointer is rewound with fseek(), and the updated struct is overwritten in-place.
When deleting, the same seek-and-overwrite technique is used to flip the is_deleted flag to 1.
This avoids rewriting the entire file and keeps file operations efficient.

---

## Exception Handling Summary

Adding Records:
- User specifies count upfront; must be a positive integer
- Each field validated individually
- Invalid records are skipped with a clear reason and the counter stays so you still add the full count
- Name, disease, and doctor cannot be empty
- Age must be 1 to 120
- Gender must be exactly Male, Female, or Other
- Contact must be exactly 10 digits
- Dates must be in YYYY-MM-DD format
- Discharge date is optional; leaving it blank means the patient is still admitted

Viewing a Record:
- If ID exists and is active, all fields are shown
- If ID was once added but is now deleted, the user is told to check the deletion log
- If ID never existed, the user is informed and asked to verify the ID

Updating a Record:
- Only active (non-deleted) and currently admitted (no discharge date) records can be updated
- Discharged patient records are locked
- Deleted records cannot be updated
- Non-existent IDs are caught with appropriate messages

Deleting a Record:
- Three valid reasons must be selected before deletion proceeds
- Reason 1 (discharged) is blocked if the patient has no discharge date set
- Confirmation with "yes" is required before deletion happens
- Every deletion is logged with reason and full timestamp to deletion_log.dat
- Already-deleted records are caught and reported clearly

File Error Handling:
- If patients.dat cannot be opened for reading, the user is told no records exist yet
- If patients.dat cannot be opened for writing, a specific error with reason is printed
- If deletion_log.dat cannot be written, a warning is shown but the program continues