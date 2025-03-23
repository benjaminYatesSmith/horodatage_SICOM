#include "ht16k33.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

 // Commands
 #define HT16K33_ON              0x21  // 0=off 1=on
 #define HT16K33_STANDBY         0x20  // bit xxxxxxx0
 
 // bit pattern 1000 0xxy
 // y    =  display on / off
 // xx   =  00=off     01=2Hz     10=1Hz     11=0.5Hz
 #define HT16K33_DISPLAYON       0x81
 #define HT16K33_DISPLAYOFF      0x80
 #define HT16K33_BLINKON0_5HZ    0x87
 #define HT16K33_BLINKON1HZ      0x85
 #define HT16K33_BLINKON2HZ      0x83
 #define HT16K33_BLINKOFF        0x81
 
 // bit pattern 1110 xxxx
 // xxxx    =  0000 .. 1111 (0 - F)
 #define HT16K33_BRIGHTNESS      0xE0
 
 //
 //  HEX codes 7 segment
 //
 //      01
 //  20      02
 //      40
 //  10      04
 //      08
 //
 const uint8_t charmap[] = {
         0x3F,   // 0
         0x06,   // 1
         0x5B,   // 2
         0x4F,   // 3
         0x66,   // 4
         0x6D,   // 5
         0x7D,   // 6
         0x07,   // 7
         0x7F,   // 8
         0x6F,   // 9
         0x77,   // A
         0x7C,   // B
         0x39,   // C
         0x5E,   // D
         0x79,   // E
         0x71,   // F
         0x00,   // space
         0x40,   // minus
         0x30,   // I
         0x38,   // L
 };
 
 volatile uint8_t _displayCache[8];   	//cache for performance
 volatile uint8_t _digits = 8;			//number of digits to display (0-4)
 volatile uint8_t _bright = 15;			//current brightness (0-15)



 //##### BEGIN: I2C-WRITE-FUNCTIONS (PRIVATE) #####
// sends complete cached data per i2c

void ecrire_digit_pos(uint8_t pos, uint8_t digit) {
    // Vérifier que la position est valide (entre 0 et 7)
    if (pos > 7) {
        return; // Position invalide, ne rien faire
    }

    // Vérifier que le chiffre est dans la plage valide (entre 0 et 9)
    if (digit > 9) {
        return; // Chiffre invalide, ne rien faire
    }

    uint8_t mask;

    // Utiliser la table charmap pour obtenir le code du chiffre
    mask = charmap[digit];

    // Envoyer la donnée via I2C à la position spécifiée
    uint8_t pData[2];
    pData[0] = pos * 2;  // Calculer la position pour l'I2C (le tableau est à 8 positions)
    pData[1] = mask;     // Le code du chiffre à afficher

    // Transmission I2C pour l'affichage
    HAL_I2C_Master_Transmit(&HT16K33_I2C_PORT, HT16K33_I2C_ADDR << 1, pData, 2, HAL_MAX_DELAY);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// exemple de trame NMEA GGA : $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47

//On veut extraire de cette trame, l'heure,minutes et secondes. 




// Fonction pour extraire l'heure d'une trame NMEA GPGGA
void extraire_heure_gga(const char* trame, uint8_t* heures, uint8_t* minutes, uint8_t* secondes) {
    // Rechercher le champ horaire après "$GPGGA,"
    char* ptr = strstr(trame, "$GPGGA");
    if (ptr != NULL) {
        // Le champ horaire est juste après la première virgule
        char heure_str[7];  // 6 caractères pour l'heure + '\0'
        sscanf(ptr, "$GPGGA,%6s", heure_str);  // Extraire l'heure sous forme de chaîne

        // Convertir la chaîne en heure, minutes, secondes
        *heures = (heure_str[0] - '0') * 10 + (heure_str[1] - '0');
        *minutes = (heure_str[2] - '0') * 10 + (heure_str[3] - '0');
        *secondes = (heure_str[4] - '0') * 10 + (heure_str[5] - '0');
    }
}

// Fonction pour afficher l'heure sur l'afficheur 7 segments
void afficher_heure(uint8_t heures, uint8_t minutes, uint8_t secondes) {
    uint8_t digits[6];

    // Extraire les chiffres de l'heure, des minutes et des secondes
    digits[0] = (heures >> 4) & 0x0F;  // Heure (premier chiffre)
    digits[1] = heures & 0x0F;         // Heure (deuxième chiffre)
    digits[2] = (minutes >> 4) & 0x0F; // Minute (premier chiffre)
    digits[3] = minutes & 0x0F;        // Minute (deuxième chiffre)
    digits[4] = (secondes >> 4) & 0x0F; // Seconde (premier chiffre)
    digits[5] = secondes & 0x0F;       // Seconde (deuxième chiffre)

    // Affichage sur l'afficheur 7 segments (en deux groupes)
    for (uint8_t i = 0; i < 3; i++) {
        ecrire_digit_pos(i, digits[i]);           // Affiche les premières trois positions (Heures)
        ecrire_digit_pos(i + 3, digits[i + 3]);   // Affiche les minutes et secondes
    }
}

// int main(void) {
//     // Exemple de trame NMEA GPGGA
//     const char* trame_nmea = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
//     // Variables pour stocker l'heure, les minutes et les secondes
//     uint8_t heures, minutes, secondes;

//     // Extraire l'heure, les minutes et les secondes de la trame NMEA
//     extraire_heure_gga(trame_nmea, &heures, &minutes, &secondes);

//     // Afficher l'heure sur l'afficheur 7 segments
//     afficher_heure(heures, minutes, secondes);

//     return 0;
// }
