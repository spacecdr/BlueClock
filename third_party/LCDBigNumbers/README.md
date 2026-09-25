# Font LCD a due righe

Fonte: https://github.com/ArminJo/LCDBigNumbers (file master scaricato il 9 settembre 2026).
Autore: Armin Joachimsmeyer. Licenza GPL-3.0-or-later, testo integrale in LICENSE.txt.

LCDBigNumbers.hpp è la copia originale di riferimento. In src/display_fonts.h sono estratte le cinque coppie di tabelle bitmap/cifre per display a due righe, con un renderer adattato al driver hd44780 già presente nel progetto. Gli altri formati e il driver della libreria non vengono compilati.

Mappatura nel pannello: Slanciato = 1x2_1; Trek = 2x2_1; Classico = 3x2_1; Arrotondato = 3x2_3; Geometrico = 3x2_2. Commenti e attribuzioni alle fonti originarie delle forme sono conservati nelle tabelle.
