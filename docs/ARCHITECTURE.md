# Logica del firmware

## Avvio e ciclo

`setup()` legge LittleFS, imposta il fuso, inizializza I²C e LCD, registra il pannello e avvia le reti salvate. `loop()` gestisce HTTP/Alexa, DNS del captive portal, Wi-Fi, NTP, LED, display e luce. I confronti dei timer sono compatibili con l'overflow di `millis()`. DNS, HTTP e I²C possono occupare temporaneamente il ciclo: non è interamente asincrono.

## Wi-Fi

La cache conserva al massimo tre coppie SSID/password dalla più recente connessione riuscita. Una candidata entra in cache solo dopo `WL_CONNECTED`: una password errata non sostituisce le credenziali valide. Ogni rete ha 15 secondi di tentativo; se tutte falliscono viene aperto l'AP con DNS verso `192.168.4.1`.

In AP il display alterna nome, password e IP ogni 3,5 secondi. Le reti salvate vengono ritentate ogni 60 secondi. Dopo il successo l'AP si chiude e l'IP della LAN viene mostrato per 2 secondi.

## Ora

La richiesta NTP UDP verifica origine, porta, modalità della risposta, stratum e token. La conversione gestisce il cambio d'era NTP nel 2036. Un successo imposta l'ora e pianifica la prossima richiesta dopo 900.000 ms; un errore la riprogramma dopo 60.000 ms.

La modalità manuale interpreta data e ora nel fuso scelto. I fusi usano regole POSIX predefinite, non un database IANA aggiornabile. L'ora continua ad avanzare senza Internet se già acquisita, ma non sopravvive alla perdita di alimentazione.

## Display e luce

`line()` confronta i 16 byte con la riga precedente, compreso il codice del carattere personalizzato zero, per evitare riscritture inutili. Le viste di connessione/AP hanno precedenza sull'orologio. La data compare dopo l'intervallo di orologio configurato e per la durata selezionata.

Gli otto slot CGRAM vengono ricaricati quando serve il banco di un font o l'accento della data. La fascia di luce valida è l'ultima già iniziata; prima della prima fascia del giorno si usa l'ultima del giorno precedente. Il livello viene rivalutato circa ogni 200 ms. AP, connessione e ora non valida forzano il 100%.

## Alexa

Espalexa condivide il server sulla porta 80. Parte dopo la connessione Wi-Fi se il flag è attivo. Il comando di luminosità converte 0–255 in 0–100%, disattiva le fasce e salva. Cambiare nome o flag dal pannello salva e programma un riavvio dopo 1,2 secondi.

Il servizio attivo non prova che un Echo abbia scoperto il dispositivo. I cambiamenti delle fasce non vengono sincronizzati continuamente verso Alexa. Le richieste Hue non riconosciute dalle altre rotte sono inoltrate a Espalexa da `onNotFound()`.

## Memoria

`/config.json` conserva reti, modalità ora, fuso, font, puntini, data, luce, fasce e Alexa. Il salvataggio scrive `/config.tmp`, chiude e rinomina. Avviene su modifica delle preferenze, connessione riuscita e comando Alexa, non a ogni secondo. File di configurazione e dump flash restano esclusi dal repository.

## API

| Metodo | Percorso | Funzione |
| --- | --- | --- |
| GET | `/` | Pannello incorporato |
| GET | `/api/status` | Stato e preferenze senza password Wi-Fi |
| GET | `/api/scan` | Scansione Wi-Fi; 202 se in corso |
| POST | `/api/wifi` | Propone SSID e password |
| POST | `/api/forget` | Rimuove una rete dalla cache |
| POST | `/api/settings` | Ora, fuso, font, puntini e data |
| POST | `/api/light` | Luce e fasce; corpo JSON |
| POST | `/api/alexa` | Abilitazione e nome |
| POST | `/api/time` | Ora manuale nel fuso selezionato |
| POST | `/api/sync` | Richiesta NTP immediata |

Le modifiche del pannello richiedono `X-Orologio: 1`, che non costituisce un login. Le API Hue usano il protocollo separato di Espalexa.
