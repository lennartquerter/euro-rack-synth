# Passive Multiple Module Jack Wiring Analysis

## Jack Count
- **Input jacks:** 2 (J1, J2)
- **Output jacks:** 6 (J3, J4, J5, J6, J7, J8)

## Two Banks of the Multiple

The circuit splits into two distinct banks, each with its own input jack and corresponding set of output jacks. The switching behavior is determined by the TN (Tip Normalling) contacts on the input jacks.

### Bank 1: J1 (Input) → J3, J4, J5
**Signal path:** Net-1 (audio signal)

- J1 pin T (Tip contact) connects to Net-1
- J2 pin TN (Normalling contact) also connects to Net-1
- J3, J4, J5 all have their pin T contacts on Net-1

**Normalling behavior:** When nothing is plugged into J2 (the second input), the TN contact on J2 allows signal from J1 to pass through to all outputs in Bank 1 (J3, J4, J5).

### Bank 2: J2 (Input) → J6, J7, J8
**Signal path:** Net-2 (audio signal)

- J2 pin T (Tip contact) connects to Net-2
- J3, J5, J6, J7, J8 all have their pin T or TN contacts wired to Bank 2's net

**Specific connections:**
- J6 pin T on Net-2
- J7 pin T on Net-2
- J8 pin T on Net-2

**Normalling behavior:** When J2 input is used, it drives all outputs in Bank 2 (J6, J7, J8). Plugging into J2 breaks the normalling from J1 because J2's T pin (carrying the new signal) is different from J1's T pin path.

## Normalling Pin Isolation

Each output jack has an independent TN (Tip Normalling) contact that doesn't connect to the main audio signal nets:

- J3 TN: Net-3 (isolated)
- J4 TN: Net-7 (isolated)
- J5 TN: Net-9 (isolated)
- J6 TN: Net-8 (isolated)
- J7 TN: Net-4 (isolated)
- J8 TN: Net-5 (isolated)

These isolated TN nets indicate that the output jack normalling contacts are not internally wired for any purpose in this design; they are available for external use (e.g., normalling from a different module input). This is a common pattern in modular synths.

## Ground
All jacks share a common ground rail (GNDREF) on their sleeve (S) contacts.
