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

volatile uint8_t _displayCache[7];   	//cache for performance
volatile uint8_t _digits = 4;//number of digits to display (0-4)
//volatile uint8_t _displayCache[7];   	//cache for performance
//volatile uint8_t _digits = 8;
volatile uint8_t _bright = 15;			//current brightness (0-15)








//##### BEGIN: I2C-WRITE-FUNCTIONS (PRIVATE) #####
// sends complete cached data per i2c
void _refresh() {
	uint8_t pData[2];

	for (uint8_t pos = 0; pos < 4; pos++) {
		pData[0] = pos * 2;
		pData[1] = _displayCache[pos];

		HAL_I2C_Master_Transmit(&HT16K33_I2C_PORT, HT16K33_I2C_ADDR << 1, pData,2, HAL_MAX_DELAY);
	}
}

// sends given command per i2c
void _writeCmd(uint8_t cmd) {
	HAL_I2C_Master_Transmit(&HT16K33_I2C_PORT, HT16K33_I2C_ADDR << 1, &cmd, 1, HAL_MAX_DELAY);
}



//##### END: I2C-WRITE-FUNCTIONS #####
//####################################
//##### BEGIN: CONTROL-FUNCTIONS #####

void seg7_init() {
	seg7_reset();
	seg7_displayOn();
}

void seg7_reset() {
	seg7_displayClear();
	seg7_clearCache();
	seg7_setBlinkRate(0);
	seg7_setDigits(4);
	seg7_setBrightness(15);
}



void seg7_clearCache() {
	for (uint8_t i = 0; i < 8; i++) {
		_displayCache[i] = SEG7_NONE;
	}
}

void seg7_refresh() {
	_refresh();
}

void seg7_displayOn() {
	_writeCmd(HT16K33_ON);
	_writeCmd(HT16K33_DISPLAYON);
	seg7_setBrightness(_bright);
}

void seg7_displayOff() {
	_writeCmd(HT16K33_DISPLAYOFF);
	_writeCmd(HT16K33_STANDBY);
}

void seg7_setBlinkRate(uint8_t value) {
	if (value > 0x03) {
		value = 0x00;
	}

	_writeCmd(HT16K33_BLINKOFF | (value << 1));
}

void seg7_setBrightness(uint8_t value) {
	if (value == _bright)
		return;

	_bright = value;

	if (_bright > 0x0F)
		_bright = 0x0F;

	_writeCmd(HT16K33_BRIGHTNESS | _bright);
}

void seg7_setDigits(uint8_t value) {
	_digits = (value > 4) ? 4 : value;
}

//#####  END: CONTROL-FUNCTIONS  #####
//####################################
//##### BEGIN: DISPLAY-FUNCTIONS #####


void seg7_displayClear() {
	uint8_t arr[4] = { SEG7_SPACE, SEG7_SPACE, SEG7_SPACE, SEG7_SPACE };
	seg7_display(arr);
	seg7_displayColon(0);
}

void seg7_displayColon(uint8_t on) {
	_writePos(2, on ? 2 : 0);
}


void _writePos(uint8_t pos, uint8_t mask) {
	if (_displayCache[pos] == mask)
		return;

	uint8_t pData[2];
	pData[0] = pos * 2;
	pData[1] = mask;

	HAL_I2C_Master_Transmit(&HT16K33_I2C_PORT, HT16K33_I2C_ADDR << 1, pData, 2, HAL_MAX_DELAY);
	_displayCache[pos] = mask;	// update value in cache
}


void seg7_display(uint8_t *array) {
	for (uint8_t i = 0; i < (4 - _digits); i++) {
		if (array[i] != 0) {
			break;
		}
		array[i] = SEG7_SPACE;
	}

	_writePos(0, charmap[array[0]]);
	_writePos(1, charmap[array[1]]);
	_writePos(3, charmap[array[2]]);
	_writePos(4, charmap[array[3]]);
}





//##### END: DISPLAY-FUNCTIONS #####




// Nos fonctions



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

void extraire_heure_gga(const char* trame, uint8_t heure_tab[6]) {
    char* ptr = strstr(trame, "$GPGGA,");
    if (ptr != NULL) {
        char heure_str[7];  // 6 caractères pour HHMMSS + '\0'
        sscanf(ptr + 7, "%6s", heure_str);  // Lire HHMMSS
        heure_str[6] = '\0';

        for (int i = 0; i < 6; i++) {
            heure_tab[i] = heure_str[i] - '0';  // Conversion ASCII → entier
        }
    }
}

void afficher_heure(uint8_t heure_tab[6]) {
    ecrire_digit_pos(0, heure_tab[0]);
    ecrire_digit_pos(1, heure_tab[1]);
    ecrire_digit_pos(2, heure_tab[2]);
    ecrire_digit_pos(3, heure_tab[3]);
    ecrire_digit_pos(4, heure_tab[4]);
    ecrire_digit_pos(5, heure_tab[5]);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Les fonctions qui suivent sont optimisées en termes de vitesse (pas de division)


//fonction qui affiche les secondes (argument de la fonction est int entre 0 et 59)
// sur l'afficheur 7 segments au position 2 & 3.

void afficher_minutes(uint8_t minutes) {
    uint8_t dizaine = minutes / 10;  // Division pour extraire la dizaine
    uint8_t unite = minutes % 10;    // Modulo pour extraire l'unité

    ecrire_digit_pos(0, dizaine);
    ecrire_digit_pos(1, unite);
}



void afficher_secondes(uint8_t secondes) {
    uint8_t dizaine = secondes / 10;  // Division pour extraire la dizaine
    uint8_t unite = secondes % 10;    // Modulo pour extraire l'unité

    ecrire_digit_pos(2, dizaine);
    ecrire_digit_pos(3, unite);
}

// Fonction qui affiche les millisecondes (valeur décimale entre 0 et 999)
// sur l'afficheur 7 segments aux positions 5, 6 et 7.
void afficher_millisec(uint16_t millis) {
    uint8_t centaine = millis / 100;                  // Centaines
    uint8_t dizaine = (millis % 100) / 10;              // Dizaines
    uint8_t unite = millis % 10;                        // Unités

    ecrire_digit_pos(5, centaine);
    ecrire_digit_pos(6, dizaine);
    ecrire_digit_pos(7, unite);
}



// Pour afficher un point sur l'afficheur 7 segments à une position donnée

void ecrire_point_pos(uint8_t pos, int afficher_point) {
    // Vérifier que la position est valide (entre 0 et 7)
    if (pos > 7) {
        return; // Position invalide, ne rien faire
    }

    uint8_t mask = 0x00; // Valeur de départ, pas de segments allumés

    if (afficher_point) {
        mask = 0x80;  // Activer le point décimal (bit 7)
    }

    // Envoyer la donnée via I2C à la position spécifiée
    uint8_t pData[2];
    pData[0] = pos * 2;  // Calculer la position pour l'I2C (le tableau est à 8 positions)
    pData[1] = mask;     // Le masque avec ou sans le point

    // Transmission I2C pour l'affichage
    HAL_I2C_Master_Transmit(&HT16K33_I2C_PORT, HT16K33_I2C_ADDR << 1, pData, 2, HAL_MAX_DELAY);
}
