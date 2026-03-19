// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// clientData structure definition
struct client_data
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure client_data

enum { MAX_RECORDS = 100 };

// prototypes
unsigned int enter_choice(void);
FILE *open_data_file(const char *path);
void initialize_file(FILE *fptr);
void list_accounts(FILE *read_ptr);
void display_account(FILE *fptr);
void text_file(FILE *read_ptr);
void update_record(FILE *fptr);
void new_record(FILE *fptr);
void delete_record(FILE *fptr);
void debit_transaction(FILE *fptr);
void write_sorted_text_file(FILE *read_ptr);
unsigned int get_account_number(const char *prompt);
double get_double(const char *prompt);
double get_positive_double(const char *prompt);
void clear_input_line(void);
int compare_names(const struct client_data *a, const struct client_data *b);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    (void)argc;
    cfPtr = open_data_file("credit.dat");
    if (cfPtr == NULL) {
        printf("%s: File could not be opened.\n", argv[0]);
        return EXIT_FAILURE;
    }

    // enable user to specify action
    while ((choice = enter_choice()) != 9)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            text_file(cfPtr);
            break;
        // update record
        case 2:
            update_record(cfPtr);
            break;
        // create record
        case 3:
            new_record(cfPtr);
            break;
        // delete existing record
        case 4:
            delete_record(cfPtr);
            break;
        // list all account information
        case 5:
            list_accounts(cfPtr);
            break;
        // search and display a single account
        case 6:
            display_account(cfPtr);
            break;
        // ATM-style debit transaction (withdraw)
        case 7:
            debit_transaction(cfPtr);
            break;
        // write a sorted text file
        case 8:
            write_sorted_text_file(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    return 0;
} // end main

FILE *open_data_file(const char *path)
{
    FILE *fPtr = fopen(path, "rb+");
    if (fPtr != NULL) {
        return fPtr;
    }

    fPtr = fopen(path, "wb+");
    if (fPtr == NULL) {
        return NULL;
    }

    initialize_file(fPtr);
    return fPtr;
}

void initialize_file(FILE *fptr)
{
    unsigned int i;
    struct client_data blank_client = {0, "", "", 0.0};

    rewind(fptr);
    for (i = 0; i < MAX_RECORDS; ++i) {
        fwrite(&blank_client, sizeof(struct client_data), 1, fptr);
    }
    fflush(fptr);
    rewind(fptr);
}

void clear_input_line(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

unsigned int get_account_number(const char *prompt)
{
    unsigned int accountNum;

    for (;;) {
        printf("%s", prompt);
        if (scanf("%u", &accountNum) == 1 && accountNum >= 1 && accountNum <= MAX_RECORDS) {
            clear_input_line();
            return accountNum;
        }
        clear_input_line();
        printf("Please enter a number from 1 to %d.\n", MAX_RECORDS);
    }
}

double get_double(const char *prompt)
{
    double value;
    for (;;) {
        printf("%s", prompt);
        if (scanf("%lf", &value) == 1) {
            clear_input_line();
            return value;
        }
        clear_input_line();
        puts("Please enter a valid number.");
    }
}

double get_positive_double(const char *prompt)
{
    double value;
    for (;;) {
        value = get_double(prompt);
        if (value > 0.0) {
            return value;
        }
        puts("Please enter a positive amount.");
    }
}

void list_accounts(FILE *read_ptr)
{
    struct client_data client = {0, "", "", 0.0};
    unsigned int count = 0;

    rewind(read_ptr);
    puts("\nAcct  Last Name        First Name      Balance");
    puts("----  --------------   ----------   ----------");

    while (fread(&client, sizeof(struct client_data), 1, read_ptr) == 1) {
        if (client.acctNum != 0) {
            printf("%-4u  %-14s   %-10s   %10.2f\n",
                   client.acctNum, client.lastName, client.firstName, client.balance);
            ++count;
        }
    }

    if (count == 0) {
        puts("(no accounts found)");
    }
}

void display_account(FILE *fptr)
{
    struct client_data client = {0, "", "", 0.0};
    unsigned int account_num = get_account_number("Enter account number to search ( 1 - 100 ): ");

    fseek(fptr, (account_num - 1) * sizeof(struct client_data), SEEK_SET);
    if (fread(&client, sizeof(struct client_data), 1, fptr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account #%u has no information.\n", account_num);
        return;
    }

    puts("\nAccount found:");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("%-6u%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);
}

// create formatted text file for printing
void text_file(FILE *read_ptr)
{
    FILE *writePtr; // accounts.txt file pointer
    // create clientData with default information
    struct client_data client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(read_ptr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (fread(&client, sizeof(struct client_data), 1, read_ptr) == 1) {
            if (client.acctNum != 0) {
                fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                        client.acctNum, client.lastName, client.firstName, client.balance);
            }
        }

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void update_record(FILE *fptr)
{
    // create clientData with no information
    struct client_data client = {0, "", "", 0.0};

    // obtain number of account to update
    unsigned int account = get_account_number("Enter account to update ( 1 - 100 ): ");

    // move file pointer to correct record in file
    fseek(fptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    // read record from file
    if (fread(&client, sizeof(struct client_data), 1, fptr) != 1) {
        puts("Error reading the record.");
        return;
    }
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        double transaction = get_double("Enter charge ( + ) or payment ( - ): ");
        if (client.balance + transaction < 0.0) {
            puts("Transaction denied: balance cannot go negative.");
            return;
        }
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fptr, -sizeof(struct client_data), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct client_data), 1, fptr);
        fflush(fptr);
    } // end else
} // end function updateRecord

// delete an existing record
void delete_record(FILE *fptr)
{
    struct client_data client;                         // stores record read from file
    struct client_data blank_client = {0, "", "", 0};  // blank client
    unsigned int account_num;                          // account number

    // obtain number of account to delete
    account_num = get_account_number("Enter account number to delete ( 1 - 100 ): ");

    // move file pointer to correct record in file
    fseek(fptr, (account_num - 1) * sizeof(struct client_data), SEEK_SET);
    // read record from file
    if (fread(&client, sizeof(struct client_data), 1, fptr) != 1) {
        puts("Error reading the record.");
        return;
    }
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", account_num);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fptr, (account_num - 1) * sizeof(struct client_data), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blank_client, sizeof(struct client_data), 1, fptr);
        fflush(fptr);
    } // end else
} // end function deleteRecord

// create and insert record
void new_record(FILE *fptr)
{
    // create clientData with default information
    struct client_data client = {0, "", "", 0.0};
    unsigned int account_num; // account number

    // obtain number of account to create
    account_num = get_account_number("Enter new account number ( 1 - 100 ): ");

    // move file pointer to correct record in file
    fseek(fptr, (account_num - 1) * sizeof(struct client_data), SEEK_SET);
    // read record from file
    if (fread(&client, sizeof(struct client_data), 1, fptr) != 1) {
        puts("Error reading the record.");
        return;
    }
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        if (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3) {
            clear_input_line();
            puts("Invalid input. Record not created.");
            return;
        }
        clear_input_line();

        client.acctNum = account_num;
        // move file pointer to correct record in file
        fseek(fptr, (client.acctNum - 1) * sizeof(struct client_data), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct client_data), 1, fptr);
        fflush(fptr);
    } // end else
} // end function newRecord

void debit_transaction(FILE *fptr)
{
    struct client_data client = {0, "", "", 0.0};
    unsigned int account = get_account_number("Enter account number for debit ( 1 - 100 ): ");
    double amount = get_positive_double("Enter debit amount (positive): ");

    fseek(fptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    if (fread(&client, sizeof(struct client_data), 1, fptr) != 1) {
        puts("Error reading the record.");
        return;
    }

    if (client.acctNum == 0) {
        printf("Account #%u has no information.\n", account);
        return;
    }

    if (client.balance - amount < 0.0) {
        puts("Debit denied: insufficient balance.");
        return;
    }

    client.balance -= amount;
    fseek(fptr, -sizeof(struct client_data), SEEK_CUR);
    fwrite(&client, sizeof(struct client_data), 1, fptr);
    fflush(fptr);
    puts("Debit successful.");
}

int compare_names(const struct client_data *a, const struct client_data *b)
{
    int last = strcmp(a->lastName, b->lastName);
    if (last != 0) {
        return last;
    }
    return strcmp(a->firstName, b->firstName);
}

void write_sorted_text_file(FILE *read_ptr)
{
    struct client_data records[MAX_RECORDS];
    unsigned int count = 0;
    unsigned int i, j;

    rewind(read_ptr);
    while (count < MAX_RECORDS &&
           fread(&records[count], sizeof(struct client_data), 1, read_ptr) == 1) {
        if (records[count].acctNum != 0) {
            ++count;
        }
    }

    if (count == 0) {
        puts("No accounts to sort.");
        return;
    }

    for (i = 0; i + 1 < count; ++i) {
        for (j = 0; j + 1 < count - i; ++j) {
            if (compare_names(&records[j], &records[j + 1]) > 0) {
                struct client_data tmp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = tmp;
            }
        }
    }

    {
        FILE *out = fopen("accounts_sorted.txt", "w");
        if (out == NULL) {
            puts("File could not be opened.");
            return;
        }

        fprintf(out, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
        for (i = 0; i < count; ++i) {
            fprintf(out, "%-6u%-16s%-11s%10.2f\n",
                    records[i].acctNum, records[i].lastName, records[i].firstName, records[i].balance);
        }
        fclose(out);
    }

    puts("Wrote sorted accounts to \"accounts_sorted.txt\".");
}

// enable user to input menu choice
unsigned int enter_choice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - list all accounts\n"
                 "6 - search account and display\n"
                 "7 - ATM debit transaction (withdraw)\n"
                 "8 - store a sorted text file \"accounts_sorted.txt\"\n"
                 "9 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1) {
        clear_input_line();
        return 0;
    }
    clear_input_line();
    return menuChoice;
} // end function enterChoice