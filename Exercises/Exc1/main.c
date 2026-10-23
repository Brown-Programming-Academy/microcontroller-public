#include <stdio.h>

// --- Setters ---

/// @brief Sets the "happy" bit of the dog byte.
/// @param dog Pointer to the byte holding the dog's attributes.
/// @param status 1 to mark the dog as happy, 0 to mark it as not happy.
/// @return 0 on success, non-zero if dog is NULL or status is invalid.
int dog_set_is_happy(unsigned char* dog, int status)
{
    return 0;
}

/// @brief Sets the "hungry" bit of the dog byte.
/// @param dog Pointer to the byte holding the dog's attributes.
/// @param status 1 to mark the dog as hungry, 0 to mark it as not hungry.
/// @return 0 on success, non-zero if dog is NULL or status is invalid.
int dog_set_is_hungry(unsigned char* dog, int status)
{
    return 0;
}

/// @brief Sets the sex bit of the dog byte.
/// @param dog Pointer to the byte holding the dog's attributes.
/// @param sex 'M' for male or 'F' for female.
/// @return 0 on success, non-zero if dog is NULL or sex is invalid.
int dog_set_sex(unsigned char* dog, unsigned char sex)
{
    return 0;
}

/// @brief Sets the age bits of the dog byte, leaving the other bits unchanged.
/// @param dog Pointer to the byte holding the dog's attributes.
/// @param new_age The new age; must fit in the bits reserved for age.
/// @return 0 on success, non-zero if dog is NULL or new_age is out of range.
int dog_set_age(unsigned char* dog, int new_age)
{
    return 0;
}

// --- Getters ---

/// @brief Reads the "happy" bit of the dog byte.
/// @param dog The byte holding the dog's attributes.
/// @return 1 if the dog is happy, 0 otherwise.
int dog_get_is_happy(unsigned char dog)
{
    return 0;
}

/// @brief Reads the "hungry" bit of the dog byte.
/// @param dog The byte holding the dog's attributes.
/// @return 1 if the dog is hungry, 0 otherwise.
int dog_get_is_hungry(unsigned char dog)
{
    return 0;
}

/// @brief Reads the sex bit of the dog byte.
/// @param dog The byte holding the dog's attributes.
/// @return 'M' if the dog is male, 'F' if female.
unsigned char dog_get_sex(unsigned char dog)
{
    return 0;
}

/// @brief Reads the age bits of the dog byte.
/// @param dog The byte holding the dog's attributes.
/// @return The dog's age.
int dog_get_age(unsigned char dog)
{
    return 0;
}

/// @brief Packs BitBite's attributes into a single byte using the setters,
///        then reads them back with the getters and prints them.
/// @return 0 on successful completion.
int main()
{
    // ==========================================
    // 1. Initialize
    // ==========================================

    unsigned char bitBite;

    // ==========================================
    // 2. Setters
    // ==========================================

    dog_set_name(&bitBite, "BitBite");
    dog_set_age(&bitBite, 7);
    dog_set_is_happy(&bitBite, 1);
    dog_set_is_hungry(&bitBite, 0);
    dog_set_sex(&bitBite, 'M');

    // ==========================================
    // 3. Getters
    // ==========================================
    
    printf("Name:      %s\n", dog_get_name(bitBite));
    printf("Age:       %d\n", dog_get_age(bitBite));
    printf("Happy:     %s\n", dog_get_is_happy(bitBite) ? "Yes" : "No");
    printf("Hungry:    %s\n", dog_get_is_hungry(bitBite) ? "Yes" : "No");
    printf("Sex:       %c\n", dog_get_sex(bitBite));
    
    return 0;
}