/*
 * sevensegment.c
 *
 *  Created on: 20.05.2022
 *      Author: lenni
 */

 #include "ht16k33.h"

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
HAL_GPIO_SetPin(GPIOA,GPIO_PIN_5);
HAL_Delay(500);

// sends value (mask) for specific position per i2c, if different from cached value
void _ecris_pos(uint8_t pos, uint8_t mask) {
	if (_displayCache[pos] == mask)
		return;

	uint8_t pData[2];
	pData[0] = pos * 2;
	pData[1] = mask;

	HAL_I2C_Master_Transmit(&HT16K33_I2C_PORT, HT16K33_I2C_ADDR << 1, pData, 2, HAL_MAX_DELAY);
	_displayCache[pos] = mask;	// update value in cache
}



###################################################################################



#define MAX_GGA_LENGTH 80

// Fonction pour extraire l'heure à partir de la chaîne NMEA GGA
void parseGGA(const char *gga, int *timeData) {
    // 1. Chercher l'indice du début de l'heure (7ème champ après GGA)
    char *token = NULL;
    char *ggaCopy = strdup(gga);  // Faire une copie de la chaîne pour ne pas modifier l'original
    int i = 0;
    token = strtok(ggaCopy, ",");
    
    // Trouver le champ de l'heure (7ème champ)
    while (token != NULL && i < 6) {
        token = strtok(NULL, ",");
        i++;
    }

    // Si l'heure est présente
    if (token != NULL) {
        // Le format de l'heure est HHMMSS, par exemple "123456" pour 12:34:56
        // Extraire l'heure, les minutes et les secondes
        int time = atoi(token);
        int hours = time / 10000;   // Extraire les heures
        int minutes = (time / 100) % 100;  // Extraire les minutes
        int seconds = time % 100;  // Extraire les secondes

        // Remplir le tableau avec les valeurs
        timeData[0] = hours;
        timeData[1] = minutes;
        timeData[2] = seconds;
    }

    // Libérer la mémoire allouée pour la copie de la chaîne
    free(ggaCopy);
}


    // Exemple de chaîne NMEA GGA
    const char *gga = "$GPGGA,123456.78,3723.247,N,12202.342,W,1,12,0.9,0.0,M,-34.1,M,,*76";