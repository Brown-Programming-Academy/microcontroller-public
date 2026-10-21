#include <stdio.h>

// --- Setters ---
int dog_set_age(unsigned char* dog, int new_age)
{
    return 0;
}

int dog_set_is_happy(unsigned char* dog, int status)
{
    return 0;
}

int dog_set_is_hungry(unsigned char* dog, int status)
{
    return 0;
}

int dog_set_sex(unsigned char* dog, unsigned char sex) 
{
    return 0;
}

// --- Getters ---

int dog_get_age(unsigned char dog)
{
    return 0;
}

int dog_get_is_happy(unsigned char dog)
{
    return 0;
}

int dog_get_is_hungry(unsigned char dog)
{
    return 0;
}

unsigned char dog_get_sex(unsigned char dog)
{
    return 0;
}

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