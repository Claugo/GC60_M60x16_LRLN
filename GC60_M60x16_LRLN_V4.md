# Aggiornamento alla versione V4 per il Modello C++

Il limite dei 64 bit viene tuttavia superato rispetto al GC60_M60x16_LRLN.cpp con l'introduzione della commutazione automatica a 128 bit che estende il calcolo ben oltre la soglia di $2^{64}$.
<br>

### **(Versione V4.0): Finestre Estese fino a $10^9$ e Certificazione a 128-Bit fino a $10^{24}$**

I test C++ illustrati nel modello GC60_M60x16_LRLN.cpp hanno valutato il comportamento del modello su finestre compatte ($W = 100.000$) entro i limiti dell'aritmetica a 64 bit. Quando però l'ampiezza della finestra cresce fino a cento milioni ($10^8$) o un miliardo di numeri ($10^9$), la gestione lineare della memoria rischia di superare la capacità della cache del processore, rallentando la fase finale di spogliatura.

Per eliminare questo collo di bottiglia ed estendere la portata numerica del motore, è stata sviluppata l'architettura **V4.0** (`GC60_M60x16_LRLN_V4.cpp`), che introduce due miglioramenti strutturali:

- **Fase C segmentata a 1-bit in cache L1:** la finestra di ricerca viene compressa a 1 bit per cella dispari e suddivisa in blocchi indipendenti da 32 KB, elaborati in parallelo da tutti i core direttamente all'interno della memoria cache più veloce della CPU.
    
- **Commutazione automatica 64/128-bit:** il programma rileva dinamicamente la magnitudo richiesta e passa in autonomia all'aritmetica a 128 bit (`__uint128_t`) con riduzione hardware nativa quando si supera la soglia di $2^{64}$.

**In questi test è stato usato un altro tipo di computer molto più datato rispetto al Ryzen 7 usato in precedenza. Questo a dimostrazione di quanto l'algoritmo lavori molto bene anche su computer un po' datati**
<br>
### Piattaforma di Test: HP Pavilion Notebook (8 Thread)

- **Processore:** Intel Core i7-6700HQ (4 core / 8 thread, architettura Skylake-S, fino a 3,5 GHz)
    
- **Cache CPU:** L1: 256 KiB | L2: 1 MiB | L3: 6 MiB
    
- **Memoria RAM:** 8 GiB (7,59 GiB disponibili)
    
- **Storage:** HDD SATA 1 TB (`HGST HTS541010A9E680`)
    
- **Sistema Operativo e Compilatore:** Linux Mint 22.3 (Kernel `7.0.0-43-generic`)  g++` 13.3.0 (`-O3 -fopenmp -march=native`)


## **Confronto Prestazionale su Finestre Ampie ($10^7$ – $10^9$)**

Per garantire un confronto rigoroso e perfettamente simmetrico, entrambi i programmi (`GC60_M60x16_LRLN_V4.cpp` e `libprimesieve`) sono stati eseguiti su tutti gli 8 thread hardware disponibili, misurando il tempo reale necessario per generare e caricare in memoria RAM l'intero elenco dei numeri primi della finestra ed escludendo la successiva scrittura dei file su disco (`finestra_1.txt` e `offset_walisch.txt`). I risultati confermano che il filtro LRLN unito alla segmentazione L1 mantiene un netto vantaggio sulle finestre mirate ($3,2\times$ su $10^7$ e $2,9\times$ su $10^8$) e supera `Primesieve` anche su un intervallo di un miliardo di numeri ($1,6\times$ su $10^9$).

---
### Tabella 1: GC60_M60x16_LRLN_V4.cpp vs Primesieve

| **Magnitudo** | **Finestra (W)** | **Primi rilevati** | **Tempo GC-60**    | **Tempo Primesieve** | **Speedup**                       |
| ------------- | ---------------- | ------------------ | ------------------ | -------------------- | --------------------------------- |
| $10^{19}$     | 10.000.000       | 229.305            | 621 ms (0,621 s)   | 1.969 ms (1,969 s)   | **GC-60 è ~3,2 volte più veloce** |
| $10^{19}$     | 100.000.000      | 2.285.232          | 803 ms (0,803 s)   | 2.365 ms (2,365 s)   | **GC-60 è ~2,9 volte più veloce** |
| $10^{19}$     | 1.000.000.000    | 22.854.258         | 2.616 ms (2,616 s) | 4.196 ms (4,196 s)   | **GC-60 è ~1,6 volte più veloce** |

### Tabella 2: Confronto Tecnico e Operativo  `M60x16 V4.0` vs `libprimesieve`

| **Aspetto confrontato**                           | **GC-60 M60x16 (V4.0)**                                                                                                                                                                                        | **libprimesieve (Kim Walisch)**                                                                                                                                         |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Logica di ricerca**                             | **Divisore-centrica:** filtra in corsa sulle 16 colonne del modulo 60 solo i divisori che colpiscono davvero la finestra (filtro LRLN).                                                                        | **Finestra-centrica:** genera tutti i numeri primi fino a $\sqrt{M}$ e li applica a blocchi sull'intervallo richiesto.                                                  |
| **Divisori elaborati** su $10^{19}$, W = $10^7$ | Genera i numeri primi di base ~149M fino a √10¹⁹, quindi applica il filtro LRLN per selezionare solo i **2.092.491** divisori che raggiungono un candidato dispari all'interno della finestra (per _p_ > _W_). | Genera i numeri primi di base ~149M fino a √10¹⁹ e li applica nell'intervallo utilizzando la segmentazione della ruota modulo-30 e la setacciatura a secchio.           |
| **Uso della memoria cache**                       | Setaccio a bit da **32 KB** (Fase B) + finestra segmentata a 1 bit in blocchi da **32 KB** dentro la cache L1 (Fase C).                                                                                        | Segmentazione a bit su modulo 30 ottimizzata per le cache L1/L2 del processore.                                                                                         |
| **Portata numerica massima**                      | **64-bit e 128-bit automatici** (`__uint128_t` con riduzione hardware `divq`, già verificato fino a $10^{24}$).                                                                                                | **Solo 64-bit** (si ferma a $\approx 1,84 \times 10^{19}$; oltre quella soglia va in overflow).                                                                         |
| **Lavoro svolto nel tempo misurato**              | Genera e carica in **memoria RAM** l'intero elenco degli offset dei primi nella finestra, fermando il cronometro prima di salvare il file su disco (`finestra_1.txt`).                                         | Genera e carica in **memoria RAM** l'intero elenco dei numeri primi nella finestra, fermando il cronometro prima di salvare gli offset su disco (`offset_walisch.txt`). |
| **Comportamento al crescere di $W$**              | Netto vantaggio sulle finestre mirate (**3,2×** su $10^7$ e **2,9×** su $10^8$) e resta davanti anche su $10^9$ (**1,6×**).                                                                                    | Riduce lo svantaggio su finestre molto ampie ($10^9$), ma resta più lento su tutte le scale testate.                                                                    |

## **Validazione Matematica Oltre i 64 Bit ($10^{20}$ – $10^{24}$)**

Superata la soglia di $2^{64}$ ($\approx 1,84 \times 10^{19}$), `Primesieve` arresta la propria operatività per limite architetturale, rendendo impossibile un confronto diretto sui tempi di esecuzione. A queste altezze, il test si concentra pertanto sulla certificazione rigorosa dell'esattezza matematica: il commutatore automatico a 128 bit (`__uint128_t`) di `GC60_M60x16_LRLN_V4.cpp` individua tutti i **989.873** numeri primi distribuiti nelle cinque finestre da $W = 10^7$ comprese tra $10^{20}$ e $10^{24}$, e ogni singolo risultato coincide al 100% con la verifica indipendente del test Baillie-PSW (BPSW). Anche in aritmetica a 128 bit, la Fase C completa la spogliatura della finestra in soli **17–18 millisecondi**.

---


### Tabella 3: Certificazione Matematica Oltre i 64-Bit GC60_M60x16_LRLN_V4.cpp` vs Test BPSW

|**Magnitudo**|**Finestra (W)**|**Divisori utili isolati**|**Primi rilevati (GC-60)**|**Primi certificati (BPSW)**|**Esito verifica**|**Tempo GC-60 (128-bit)**|
|---|---|---|---|---|---|---|
|$10^{20}$|10.000.000|2.350.497|216.979|216.979|**100% Esatto**|2 sec. 585 ms (2,585 s)|
|$10^{21}$|10.000.000|2.595.934|206.889|206.889|**100% Esatto**|8 sec. 5 ms (8,005 s)|
|$10^{22}$|10.000.000|2.827.187|197.136|197.136|**100% Esatto**|25 sec. 403 ms (25,403 s)|
|$10^{23}$|10.000.000|3.050.170|188.571|188.571|**100% Esatto**|1 min. 22 sec. 849 ms|
|$10^{24}$|10.000.000|3.264.031|180.298|180.298|**100% Esatto**|4 min. 54 sec. 239 ms|
<br>

**Nota per la versione V4.0 (`GC60_M60x16_LRLN_V4.cpp`):** il programma accetta direttamente anche la notazione esponenziale compatta per i parametri di Magnitudo e Finestra, attivando in automatico la modalità a 64 o 128 bit:

Compilato su Linux: g++ -O3 -fopenmp -march=native GC60_M60x16_LRLN_V4.cpp -o GC60_M60x16_LRLN_V4

./GC60_M60x16_V4 10^19 10^9

./GC60_M60x16_V4 10^24 10^7
