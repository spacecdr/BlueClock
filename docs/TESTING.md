# Verifica e collaudo

## Test automatici

`clock_logic_test.cpp` usa la stessa logica del firmware per fasce non ordinate, passaggi esatti, mezzanotte, singola fascia a zero, fallback, overflow dei timer e cambio d'era NTP.

`display_test.cpp` verifica cinque font: dieci cifre distinte ciascuno, indici validi, 43.200 frame di orario/modalità/fase dei puntini e limiti del buffer 16×2. Non misura la leggibilità ottica.

La CI compila il firmware per D1 mini e verifica anche la sintassi JavaScript del pannello.

## Prototipo

Sono stati verificati identificazione della flash da 4 MB, caricamento con hash, LCD I²C, LittleFS, avvio AP, connessione LAN, stato NTP aggiornato, pannello e salvataggio delle opzioni via HTTP. I controlli successivi al riavvio hanno confermato persistenza di font, puntini e configurazione Alexa.

Questi controlli non equivalgono a una prova vocale su ogni Echo. Non sono documentati un test continuativo di 15 minuti del timer NTP, una rotazione completa fra tre router o una misura strumentale della retroilluminazione.

## Prova pratica

1. Avvio senza reti disponibili: lampeggio LED e istruzioni AP.
2. Password errata, poi corretta: errore, recupero e IP per 2 secondi.
3. Tre reti e una quarta: riavvio, ordine e sostituzione della meno recente.
4. NTP, assenza di Internet, ora manuale e fusi.
5. Otto font, puntini e passaggio alla data con/senza anno.
6. Fasce vicine all'ora corrente e oltre mezzanotte; luce 0%, intermedia, 100%.
7. Scoperta Alexa sul proprio Echo e comandi vocali; passaggio alla luce fissa.
8. Spegnimento: persistenza delle preferenze e riacquisizione dell'ora.

Font, LED ed eventuale sfarfallio richiedono osservazione fisica.
