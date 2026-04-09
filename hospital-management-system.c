# include <stdio.h>
int main()
{
    printf("Welcome to the Hospital Management System\n");
    printf("Please select an option:\n");
    printf("1. Add a new patient\n");
    printf("2. View patient records\n");
    printf("3. Update patient information\n");
    printf("4. Delete a patient record\n");
    printf("5. Exit\n");

    int choice;
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            printf("Adding a new patient...\n");
            // Code to add a new patient
            break;
        case 2:
            printf("Viewing patient records...\n");
            // Code to view patient records
            break;
        case 3:
            printf("Updating patient information...\n");
            // Code to update patient information
            break;
        case 4:
            printf("Deleting a patient record...\n");
            // Code to delete a patient record
            break;
        case 5:
            printf("Exiting the system. Goodbye!\n");
            return 0;
        default:
            printf("Invalid option. Please try again.\n");
    }

    return 0;
}