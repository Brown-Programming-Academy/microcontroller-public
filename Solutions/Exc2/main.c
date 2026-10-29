#include <stdio.h>

// ==========================================
// Bit layout of the dog byte (8 bits)
// ==========================================
//
//   bit:   7    6    5    4    3    2    1        0
//        +-----+------------------------+-------+--------+
//        | sex |          age           | happy | hungry |
//        +-----+------------------------+-------+--------+
//
//   bit 0     - hungry (1 = yes, 0 = no)
//   bit 1     - happy  (1 = yes, 0 = no)
//   bits 2-6  - age    (5 bits -> values 0..31)
//   bit 7     - sex    (1 = female, 0 = male)

// Position of each field = how many places we shift to reach it
#define HUNGRY_BIT  0
#define HAPPY_BIT   1
#define AGE_SHIFT   2
#define SEX_BIT     7

// Masks: a mask has 1s exactly where the field sits, and 0s everywhere else
#define HUNGRY_MASK (1 << HUNGRY_BIT)      // 0000 0001
#define HAPPY_MASK  (1 << HAPPY_BIT)       // 0000 0010
#define AGE_MAX     31                     // 5 bits -> 2^5 - 1 = 31
#define AGE_MASK    (AGE_MAX << AGE_SHIFT) // 0111 1100
#define SEX_MASK    (1 << SEX_BIT)         // 1000 0000

// --- Setters ---
// Every setter follows the same two steps:
//   1. Clear the field:  dog &= ~MASK   (AND with the inverted mask zeroes only the field's bits)
//   2. Write new value:  dog |= value   (OR turns on the bits we want, leaves the rest untouched)
// Each setter returns 0 on success and -1 if the input is invalid.

int dog_set_age(unsigned char* dog, int new_age)
{
    // Age must fit in 5 bits
    if (new_age < 0 || new_age > AGE_MAX)
    {
        return -1;
    }

    // Clear the old age (bits 2-6), then shift the new age into place and OR it in.
    // Example: age 7 = 0000 0111 -> shifted left by 2 -> 0001 1100
    *dog &= ~AGE_MASK;
    *dog |= (unsigned char)(new_age << AGE_SHIFT);

    return 0;
}

int dog_set_is_happy(unsigned char* dog, int status)
{
    if (status)
    {
        *dog |= HAPPY_MASK;     // Turn bit 1 on
    }
    else
    {
        *dog &= ~HAPPY_MASK;    // Turn bit 1 off
    }

    return 0;
}

int dog_set_is_hungry(unsigned char* dog, int status)
{
    if (status)
    {
        *dog |= HUNGRY_MASK;    // Turn bit 0 on
    }
    else
    {
        *dog &= ~HUNGRY_MASK;   // Turn bit 0 off
    }

    return 0;
}

int dog_set_sex(unsigned char* dog, unsigned char sex)
{
    if (sex == 'F')
    {
        *dog |= SEX_MASK;       // Female -> bit 7 = 1
    }
    else if (sex == 'M')
    {
        *dog &= ~SEX_MASK;      // Male -> bit 7 = 0
    }
    else
    {
        return -1;              // Unknown value - leave the byte unchanged
    }

    return 0;
}

// --- Getters ---
// To read a field:
//   1. AND with the mask - keeps only the field's bits, zeroes everything else
//   2. Shift right       - moves the field down to bit 0 so we get its real value

int dog_get_age(unsigned char dog)
{
    // Example: 0001 1110 & 0111 1100 = 0001 1100 -> shifted right by 2 -> 0000 0111 = 7
    return (dog & AGE_MASK) >> AGE_SHIFT;
}

int dog_get_is_happy(unsigned char dog)
{
    // Result is 1 if bit 1 is on, 0 otherwise
    return (dog & HAPPY_MASK) >> HAPPY_BIT;
}

int dog_get_is_hungry(unsigned char dog)
{
    // Bit 0 is already in the lowest position, so no shift is needed
    return dog & HUNGRY_MASK;
}

unsigned char dog_get_sex(unsigned char dog)
{
    // A non-zero result means bit 7 is on -> female
    return (dog & SEX_MASK) ? 'F' : 'M';
}

int main()
{
    // ==========================================
    // 1. Initialize
    // ==========================================

    // Start with all bits at 0 so we don't read garbage memory
    unsigned char bitBite = 0;

    // ==========================================
    // 2. Setters
    // ==========================================

    dog_set_age(&bitBite, 7);
    dog_set_is_happy(&bitBite, 1);
    dog_set_is_hungry(&bitBite, 0);
    dog_set_sex(&bitBite, 'M');

    // ==========================================
    // 3. Getters
    // ==========================================

    printf("Age:       %d\n", dog_get_age(bitBite));
    printf("Happy:     %s\n", dog_get_is_happy(bitBite) ? "Yes" : "No");
    printf("Hungry:    %s\n", dog_get_is_hungry(bitBite) ? "Yes" : "No");
    printf("Sex:       %c\n", dog_get_sex(bitBite));

    return 0;
}
