## changes.md — What I changed and why (detailed)

This document explains every change made to `trans.c`, in beginner-friendly terms.

---

## 1) What the original program was doing

The program stores **100 fixed-size records** in a binary file called `credit.dat`.

- Each record has:
  - account number (`acctNum`)
  - last name
  - first name
  - balance
- The file is treated like an array of 100 records:
  - record 1 is at position 0
  - record 2 is at position 1
  - ...
  - record 100 is at position 99

To jump to record \(n\), the program uses:

\[
 \text{byteOffset} = (n - 1) \times sizeof(struct\ clientData)
\]

Then it uses `fseek` to move the file pointer to that offset.

---

## 2) Menu changes (new options)

### 2.1 Added “list all accounts”

**What I added**
- A new menu item:
  - `5 - list all accounts`
- A new function:
  - `listAccounts(FILE *readPtr)`

**Why**
- Your `README.md` task list suggests adding functionality like listing all accounts.
- Listing records is a basic feature that fits the project theme and is still simple C.

### 2.2 Added “search account and display”

**What I added**
- Another menu item:
  - `6 - search account and display`
- A new function:
  - `displayAccount(FILE *fPtr)`

**Why**
- It demonstrates correct random access (`fseek + fread`) without changing data.
- It’s useful for testing and demos (you can quickly confirm an account exists).

### 2.3 Shifted the “end program” choice number

Originally:
- `5 - end program`

Now (after adding more features):
- `9 - end program`

**Why**
- We inserted new options (5 and 6), so exit moved to 7.

---

## 3) File opening + first-run setup (important)

### 3.1 Added `openDataFile("credit.dat")`

**Original behavior**
- Program tried:
  - `fopen("credit.dat", "rb+")`
- If the file didn’t exist, it printed an error and exited.

**New behavior**
- `openDataFile` does this:
  1. Try `rb+` (read/write existing file).
  2. If that fails, create it with `wb+` (write/read new file).
  3. When creating a new file, call `initializeFile` to fill it with 100 blank records.

**Why this matters**
- Without initialization, random-access reads can fail or read garbage, because the file may be empty.
- With initialization, record positions 1..100 always exist in the file.

### 3.2 Added `initializeFile(FILE *fPtr)`

**What it does**
- Writes 100 “blank” records:
  - `acctNum = 0`
  - names empty strings
  - `balance = 0.0`

**Why**
- The program uses `acctNum == 0` to mean “empty slot”.
- Writing 100 blank records makes all account slots valid and predictable.

---

## 4) Fix: removed the `feof()` bug in `textFile`

### 4.1 What was wrong

Original pattern:

- `while (!feof(readPtr)) { fread(...); ... }`

This is a classic mistake because:
- `feof()` becomes true **only after** you try to read past the end.
- That means the loop often runs one extra time.

So you can get:
- duplicate last record
- printing a record that wasn’t properly read

### 4.2 What I changed

Now it does:
- `while (fread(...) == 1) { ... }`

**Why this is correct**
- It loops only when a record was actually read successfully.

---

## 5) Input safety + validation (beginner-friendly)

### 5.1 Added `clearInputLine()`

**Problem**
- If the user types letters (like `abc`) when the program expects a number, `scanf` fails.
- When `scanf` fails, the bad characters stay in the input buffer.
- Then the next `scanf` fails again (infinite “stuck” behavior).

**Fix**
- `clearInputLine()` reads characters until newline (or EOF), throwing away the rest of the line.

### 5.2 Added `getAccountNumber(prompt)`

**What it does**
- Repeatedly asks until the user enters a valid account number 1..100.

**Why**
- Prevents invalid `fseek` offsets.
- Stops users from entering 0 or 999 and corrupting reads/writes.

### 5.3 Added `getDouble(prompt)`

**What it does**
- Repeatedly asks until a valid number is entered.

**Why**
- Makes `updateRecord` safer if the user types something non-numeric.

---

## 6) `scanf` format fixes (correctness)

### 6.1 Unsigned integers should use `%u`

In multiple places the original code did:
- `scanf("%d", &accountNum);`

But:
- `accountNum` is `unsigned int`

**Why this matters**
- `%d` expects an `int*`
- `%u` expects an `unsigned int*`
- Using the wrong format is undefined behavior (it may “seem fine” but it’s not guaranteed).

**What I changed**
- Wherever we read menu choice or account numbers, we now use `%u`.

---

## 7) Read/write error checks (basic robustness)

### 7.1 Checked return values from `fread`

**Original behavior**
- Code called `fread(...)` and assumed it succeeded.

**New behavior**
- After `fread`, the code checks:
  - `if (fread(...) != 1) { puts("Error reading the record."); return; }`

**Why**
- If the file is corrupted, or the disk has issues, or the pointer is wrong, `fread` can fail.
- Checking return values avoids using uninitialized data.

### 7.2 Added `fflush(fPtr)` after writes

**Where**
- After `fwrite` in:
  - `newRecord`
  - `updateRecord`
  - `deleteRecord`
  - `initializeFile`

**Why**
- `fflush` forces buffered output to be written to disk.
- This makes it more reliable that changes are saved immediately (useful during demos/testing).

---

## 8) New function details

### 8.1 `listAccounts(FILE *readPtr)`

**What it does**
- `rewind(readPtr)` to start at the beginning of the file.
- Loops through all 100 records using:
  - `while (fread(...) == 1)`
- Prints only records where:
  - `client.acctNum != 0`

**Why**
- Account number 0 is the “empty slot” marker.
- This prints only real accounts.

### 8.2 `displayAccount(FILE *fPtr)`

**What it does**
- Asks for an account number (1..100).
- `fseek` to the exact record.
- Reads it with `fread`.
- If `acctNum == 0`, prints “no information”.
- Otherwise prints that one record in a nice table format.

**Why it’s a good mini-project feature**
- Shows you understand:
  - random-access file indexing
  - reading a single record
  - checking if the slot is empty

### 8.3 ATM-style debit transaction (withdraw)

**What I added**
- `debit_transaction(FILE *fptr)` as a separate menu option.

**What it does**
- Asks for:
  - account number
  - debit amount (must be positive)
- Reads the record.
- Checks:
  - account exists (`acctNum != 0`)
  - balance will not go below 0 after debit
- If valid, subtracts and writes the record back.

**Why**
- Your guideline sheet lists “ATM debit card transaction” as an example faculty-specified feature.
- This is a realistic banking action and easy to explain in a demo.

### 8.4 Sorted output file (`accounts_sorted.txt`)

**What I added**
- `write_sorted_text_file(FILE *read_ptr)`
- A simple name comparator `compare_names(...)`

**What it does**
1. Reads all non-empty records into an array.
2. Uses **bubble sort** to sort by:
   - last name, then first name
3. Writes a formatted file called:
   - `accounts_sorted.txt`

**Why**
- The guideline sheet also lists “sorting of records using algorithm of choice”.
- Bubble sort is beginner-friendly and acceptable for only up to 100 records.

---

## 9) Minor cleanups in `main`

### 9.1 Return values

**Original**
- `main` didn’t explicitly return a value.

**Now**
- On success: `return 0;`
- On failure to open file: `return EXIT_FAILURE;`

**Why**
- Returning a status code is standard C practice and helps automated testing.

### 9.2 `(void)argc;`

**What**
- `argc` isn’t used, so the code marks it as intentionally unused.

**Why**
- Prevents “unused parameter” warnings when compiling with stronger warning flags.

---

## 10) Exactly what to say in a demo (simple explanation)

- **Random access idea**: “Account number \(n\) maps to record \((n-1)\) in the file. I compute the offset and use `fseek`.”
- **Empty record idea**: “If `acctNum == 0`, that slot is empty.”
- **Why I removed `feof`**: “`feof` becomes true only after reading past end-of-file, so it can cause an extra loop. Using `fread(...) == 1` is correct.”
- **Why I added input functions**: “If the user types wrong input, `scanf` fails; I clear the input line and ask again to avoid infinite loops.”

---

## 11) Guideline rubric alignment (from the provided page)

Based on the guideline page you shared, here is how the current code matches it:

- **Code improvement (style)**:
  - I converted function names to **snake_case** (example: `updateRecord` → `update_record`).
- **Functional decomposition**:
  - Input is separated into helper functions (`get_account_number`, `get_double`, `get_positive_double`).
  - The “work” is separated into feature functions (`list_accounts`, `display_account`, `debit_transaction`, etc.).
- **Memory usage**:
  - Uses fixed arrays and fixed-size records; sorted output uses at most 100 records in memory.
- **Processing efficiency**:
  - Correct record reading loops (`fread(...) == 1`) avoids extra iterations/bugs.
  - Random access still uses direct `fseek` to the record (O(1) access by account number).
- **Faculty-specified innovation examples**:
  - **Limit on negative balance**: enforced in `update_record` and `debit_transaction`.
  - **ATM debit transaction**: implemented (`debit_transaction`).
  - **Sorting records**: implemented (`accounts_sorted.txt`).


