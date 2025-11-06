# Libreria C personale — To Do funzionalità

Questa lista è pensata come roadmap per costruire una libreria C riutilizzabile (la tua "mini standard library").  
Le sezioni sono organizzate per aree di utilità.

---

## 1. Gestione stringhe e memoria di base
- Copia sicura di stringhe (str_copy con limite massimo)
- Concatenazione sicura di stringhe (str_concat con limite massimo)
- Rimozione spazi iniziali/finali (trim)
- Suddivisione di una stringa su delimitatore (split)
- Sostituzione di sottostringa all’interno di una stringa (replace)
- Conversione stringa → int / long / double con gestione degli errori
- "String builder" dinamico con buffer che cresce automaticamente in append

---

## 2. Utilità memoria
- Wrapper per malloc / calloc / realloc con controllo errore centralizzato
- Funzione per "alloca e azzera struct T"
- Funzione per duplicare un blocco di memoria
- Funzione per liberare in modo sicuro e impostare il puntatore a NULL
- Semplice memory arena / pool allocator (blocchi grandi da cui si prendono chunk piccoli)

---

## 3. Collezioni di base
- Array dinamico (vector): push/pop, get/set, resize automatico
- Lista collegata (singola e/o doppia)
- Stack (LIFO)
- Queue (FIFO circolare)
- Hash map (chiave stringa → void*)
- Set (insieme senza duplicati, basato su hash map)
- Ring buffer (buffer circolare per log o stream di dati)

---

## 4. Math / utilità numeriche
- Funzione clamp per forzare un valore dentro [min,max]
- Funzioni min / max sicure
- Media, varianza, deviazione standard su array di numeri
- Interpolazione lineare
- Conversioni tra tipi numerici e stringa (es. int32 <-> stringa decimale <-> stringa esadecimale)

---

## 5. Gestione file e I/O
- Leggere l’intero contenuto di un file in memoria (buffer + size)
- Scrivere un buffer/stringa su file (in modalità sovrascrivi o append)
- Leggere file riga per riga in modo semplice tipo next_line(...)
- Controllare se un file esiste
- Ottenere la dimensione di un file
- Parsing di file di configurazione stile chiave=valore

---

## 6. Logging e diagnostica
- Logger con livelli (DEBUG / INFO / WARN / ERROR)
- Logger con timestamp
- Logger che può scrivere sia su stdout che su file
- Macro di ASSERT personalizzata (stampa messaggio utile e termina)
- Funzione per fare hexdump di un buffer (debug di dati binari)

---

## 7. Tempo e timing
- Ottenere timestamp corrente in millisecondi / microsecondi
- Misurare la durata di un blocco di codice (timer start/stop)
- Sleep in millisecondi in modo portabile

---

## 8. Parsing argomenti / configurazione da CLI
- Parser per argomenti da riga di comando:
  - Flag booleani (--verbose)
  - Parametri con valore (--port 8080)
- Funzione che stampa automaticamente un messaggio di help/usage

---

## 9. Utility generiche
- Conversione stringa tutta minuscola / tutta maiuscola
- Verificare se una stringa è numerica / alfanumerica / esadecimale
- Generare numeri casuali in un intervallo
- Generare un identificatore pseudo-unico stile UUID
- Utility sui path: unire directory + filename con il separatore giusto per il sistema

---

## 10. Strutture dati avanzate
- Albero di ricerca binario / albero bilanciato (AVL, red-black, ecc.)
- Priority queue / min-heap / max-heap
- Rappresentazione di grafo (nodi, archi) e algoritmi base tipo BFS e DFS

---

## 11. Formattazione testo
- Mini printf personalizzato (studio/controllo)
- Funzione "safe format in buffer" con lunghezza massima garantita
- Funzione per creare stringhe tipo template "Hello %s x=%d"

---

## 12. Networking (livello base)
- Aprire una socket TCP client
- Aprire una socket TCP server (bind / listen / accept)
- Inviare e ricevere dati con timeout
- Funzione tcp_send_all(...) (invia tutto il buffer finché non è tutto scritto)
- Conversioni indirizzo IP ↔ stringa

---

## 13. Threading e concorrenza
- Wrapper portabile per creare thread
- Wrapper per mutex
- Spinlock semplice
- Coda thread-safe per passare messaggi tra thread
- Worker pool (pool di thread che consumano job da una coda condivisa)

---

## 14. Gestione errori centralizzata
- Enum dei codici di errore (es. OK, INVALID_ARG, OUT_OF_MEMORY, IO_ERROR, ...)
- Funzione per mappare codice errore → stringa leggibile
- Pattern: la funzione ritorna un codice di errore e scrive il risultato vero in un parametro out
- Macro per salto pulito a sezione di cleanup (goto cleanup)
- Cleanup ordinato delle risorse aperte (file, memoria, lock, socket, …)

---

## 15. Mini framework di test
- Macro tipo TEST_ASSERT(...)
- Runner che esegue tutti i test registrati e riporta pass/fail
- Conteggio test totali / passati / falliti
- Report finale riassuntivo

---

## 16. Sicurezza / robustezza
- Funzioni per gestire stringhe in modo sicuro e prevenire buffer overflow
- Sanitizzazione input esterno (rimozione caratteri indesiderati/pericolosi)
- Cancellazione sicura di buffer sensibili (es. password) prima di free
- Validatori per path (controllo di .., percorsi assoluti/relativi consentiti, ecc.)

---

## 17. Configurazione di build e portabilità
- Header centrale config.h con:
  - Macro per sistema operativo (WIN32, POSIX, ecc.)
  - Macro per keyword tipo INLINE, EXPORT, ecc.
  - Macro per abilitare/disabilitare singole feature (MYLIB_ENABLE_LOG, ecc.)
- Versione della libreria (major.minor.patch)
- Convenzione su simboli pubblici vs interni (cosa fa parte dell’API stabile)

---

## 18. Documentazione e stile
- Convenzione di naming delle funzioni (mylib_<sottosistema>_<azione>())
- Regole chiare su chi possiede / deve liberare la memoria
- Commenti standard per ogni funzione: cosa fa, parametri, valore di ritorno, possibili errori
- Changelog delle feature quando aggiungi/modifichi parti della libreria

---
