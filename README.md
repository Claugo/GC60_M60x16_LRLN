## Progetto GC-60: L'Evoluzione del Setaccio Verticale (Modulo 60)

Questo repository rappresenta probabilmente il traguardo più significativo dell'intero **progetto GC-60**. Qui viene messo in evidenza il salto logico fondamentale della mia ricerca: il passaggio da una singola intuizione sequenziale (a binario unico) a una struttura a **colonne parallele indipendenti**.

## Le Origini: Il Binario Unico

Inizialmente, il modello GC-60 si sviluppava su un unico binario. Rispetto ai crivelli tradizionali, la differenza principale era l'approccio "orizzontale", caratterizzato da uno slittamento del riferimento numerico in avanti di 10:

`10 + [1, 3, 7, 9, 13, 19, 21, 27, 31, 33, 37, 39, 43, 49, 51, 57]` $\rightarrow$ che generano `11, 13, 17, 19...`

Questo approccio ha permesso di identificare una proprietà straordinaria: una **ruota Modulo 60 totalmente passiva**, una struttura geometrica che si ripete all'infinito senza mai modificare il proprio stato.

## I Primi Programmi e il Limite dell'Archivio

Su questo principio sono nati i primi due programmi principali del progetto MicroPrime: `microprime_crea` (con il compito di costruire l'infrastruttura informativa) e `microprime_studio` (dedicato all'esplorazione delle finestre di ricerca).

Questi software sono stati i primi a superare, seppur di poco, la soglia dei 64 bit in tempi ragionevoli. Tuttavia, presentavano un grande limite pratico: per funzionare, era necessario creare e consultare un archivio statico di soli numeri primi. Questo database diventava via via sempre più pesante e difficile da gestire, specialmente superando la soglia di 10^13 - 1, che richiedeva la memorizzazione di oltre 334 miliardi di numeri primi.

## Il Salto Logico

Questo limite, anziché essere un punto d'arresto, ha stimolato un ragionamento più complesso e affascinante. Questo primo tassello si stava incastrando talmente bene con le osservazioni successive che la conclusione logica è arrivata quasi in modo spontaneo.

Il cambiamento fondamentale risiede nella transizione strutturale della ruota:

**Da una stringa a binario unico traslata di 10:**

```
10[1, 3, 7, 9, 13, 19, 21, 27, 31, 33, 37, 39, 43, 49, 51, 57]
```

**A una struttura geometrica parallela a 16 colonne indipendenti:**

$$ 
\large\overbrace{ 1\quad 7\quad 11\quad 13\quad 17\quad 19\quad 23\quad 29\quad 31\quad 37\quad 41\quad 43\quad 47\quad 49\quad 53\quad 59 }^{\mathrm{mod}\ 60}
$$

## Dall'Orizzontale al Verticale: Le Colonne Parallele

Se la struttura precedente si sviluppava in senso orizzontale, questa nuova architettura chiede di immaginare lo spazio numerico in senso **verticale**. I numeri si distribuiscono su colonne parallele autonome: ogni colonna accoglie ed elabora in totale isolamento esclusivamente i propri multipli e i propri numeri primi.

Il passaggio dal binario unico del Modulo 60 a questa spogliatura a colonne indipendenti non è stato immediato, ma ha attraversato diverse implementazioni intermedie, ciascuna delle quali ha aggiunto un tassello fondamentale alla ricerca:

- **GC60-M30x3_suffix**: Una versione intermedia capace di esplorare finestre ad alta magnitudo come $10^{27}$ in poco più di 3 ore.
    
- **Microprime_M60_suffix**: Una suite di programmi dedicata allo studio approfondito dei numeri primi, arricchita con grafici, statistiche e strumenti di analisi per verificare teorie matematiche complesse, come la congettura di Hardy-Littlewood sulla densità dei primi e dei primi gemelli.
    
- **LRLN (Local Residue Lattice Navigation)**: Sviluppato nelle ultime settimane, questo algoritmo di _Navigazione su Reticoli di Residui Locali_ introduce un filtro geometrico stringente in corsa. I candidati vengono scremati all'istante, lasciando passare solo i divisori che soddisfano specifiche proprietà simmetriche rispetto alla finestra e riducendo drasticamente lo sforzo della CPU.
    

_Ognuno di questi passaggi è documentato in repository e documenti indipendenti che potete consultare; creazioni uniche che condividono una logica di fondo mano a mano sempre più performante_.

## Lo Scopo di Questo Repository

Lo scopo di questo repository **non è presentare l'ennesima variante** di un crivello segmentato tradizionale. L'obiettivo è dimostrare, attraverso argomentazioni matematiche e verifiche sperimentali, una **logica originale per l'organizzazione dei numeri**.

Questa architettura consente di separare i numeri secondo una struttura ben definita, semplificando lo scorrimento dei multipli e offrendo un punto di osservazione alternativo sul modo in cui i numeri primi emergono all'interno della scala numerica.

### I Limiti del Confronto

In questo documento verranno presentati due distinti livelli di confronto:

- **Il primo (Julia vs Primesieve):** sarà limitato a intervalli entro i 64 bit. L'unica eccezione sarà un breve test a magnitudini superiori, utilizzato esclusivamente per dimostrare che il modello logico non è intrinsecamente vincolato da questo limite.
    
- **Il secondo (C++ vs Primesieve):** questo confronto è stato introdotto per una finalità puramente matematica. Poiché Primesieve opera a 64 bit sfruttando al massimo le potenzialità delle architetture moderne, l'obiettivo era lavorare nelle stesse identiche condizioni computazionali, limitando l'esecuzione dello script `GC60_M60x16_LRLN.cpp` ai soli 64 bit.
    

L'obiettivo del benchmark non è stabilire quale programma sia in assoluto il più veloce, ma misurare il comportamento del metodo rispetto al riferimento più autorevole oggi disponibile. Da molti anni, infatti, _Primesieve_ di Kim Walisch rappresenta lo stato dell'arte dei crivelli segmentati. Le sue prestazioni sono il risultato di un'eccellente implementazione in C++ e di un lungo lavoro di ottimizzazione algoritmica che lo ha reso il punto di riferimento mondiale.


### Il Paradosso Sperimentale: Julia vs primesieve

Per questo motivo, il confronto assume un significato particolare. Non vengono messi a confronto due programmi scritti nello stesso linguaggio, né due implementazioni sviluppate con lo stesso livello di maturità del codice:

- Da una parte vi è **Primesieve**, scritto in C++ e perfezionato nel corso degli anni da una vasta comunità.
    
- Dall'altra un programma sviluppato in pochi mesi in **Julia**, un linguaggio di alto livello che, a parità di algoritmo, non può competere linearmente con le prestazioni offerte da un'implementazione C++ accuratamente ottimizzata a basso livello.
    

Proprio per questo, qualora il programma presentato in questo repository riesca a eguagliare o superare le prestazioni di Primesieve, **il risultato non potrà essere attribuito al linguaggio di programmazione**. Al contrario, in questo test Julia rappresenta una condizione di partenza volutamente sfavorevole.

### Il Vantaggio della Logica

Entrambi i programmi operano nello stesso dominio matematico e perseguono il medesimo obiettivo: individuare i numeri primi mediante un crivello segmentato. Se l'algoritmo GC-60 ottiene prestazioni superiori pur partendo da uno svantaggio implementativo evidente, significa che la differenza risiede necessariamente nella **logica con cui il problema viene affrontato**.

In altre parole, questo confronto non vuole dimostrare che Julia sia più veloce del C++, ma verificare se una diversa organizzazione matematica del problema possa produrre un vantaggio tale da compensare, e persino superare, lo svantaggio imposto dall'implementazione.

Se ciò accade, la conclusione è naturale: **il vantaggio appartiene al metodo, non al linguaggio**. Di conseguenza, una futura implementazione dello stesso metodo in C++, sviluppata con il medesimo livello di ottimizzazione raggiunto da Primesieve, rappresenterebbe un ulteriore passo avanti e non un semplice pareggio tecnologico.

## Le Colonne come Binari Verticali: Un Esempio Pratico

Per comprendere come i divisori si muovano lungo le colonne e come avvenga la loro mappatura, analizziamo il comportamento di un numero primo specifico: $p = 11$.

> 💡 **Nota fondamentale sui segmenti:** Quando il calcolo parte dall'origine (ovvero da zero), il primo numero composto da analizzare ed eliminare per qualsiasi numero primo $p$ è sempre il suo quadrato ($p^2$). Al contrario, quando l'algoritmo esplora una finestra remota ad alta magnitudo, l'aggancio iniziale non avverrà sul quadrato, ma sul primo multiplo utile del primo che cade all'interno di quel segmento. Questa differenza non incide sulla struttura geometrica: per ogni nuovo segmento, il punto di aggancio viene ricalcolato istantaneamente.

Prendiamo come riferimento la partenza dall'origine per il numero **$p = 11$** (il cui quadrato è $121$). Se mappiamo i primi 16 multipli successivi espressi nel Modulo 60, otteniamo una distribuzione sorprendente:

|**Multiplo di 11**|**Colonna (Resto Modulo 60)**|**Riga (Indice del Ciclo)**|
|---|---|---|
|**121** (Quadrato)|1|2|
|**143**|23|2|
|**187**|7|3|
|**209**|29|3|
|**253**|13|4|
|**319**|19|5|
|**341**|41|5|
|**407**|47|6|
|**451**|31|7|
|**473**|53|7|
|**517**|37|8|
|**539**|59|8|
|**583**|43|9|
|**649**|49|10|
|**671**|11|11|
|**737**|17|12|

### La Legge di Decompressione Numerica

Qualsiasi multiplo presente in tabella può essere ricostruito matematicamente partendo dalle sue coordinate geometriche (Riga e Colonna) attraverso una semplice equazione lineare:

$$\text{Multiplo} = (\text{Riga} \times 60) + \text{Colonna}$$

Ogni valore risultante appartiene in modo **esclusivo e univoco** a quella specifica colonna e non comparirà mai in nessun'altra delle restanti 15.

- _Esempio:_ Prendiamo la riga 9 sulla colonna 43:
    
    $$(9 \times 60) + 43 = 583 \quad \longrightarrow \quad 583 \pmod{11} = 0$$
    

### La Svolta Computazionale: Avanzamento Verticale per Addizione

Nei crivelli tradizionali, per eliminare i multipli di 11 bisogna avanzare linearmente nella memoria di 11 in 11. Nella struttura a colonne di GC-60, invece, avviene una magia geometrica: il multiplo successivo di 11 _all'interno della stessa identica colonna_ si trova semplicemente **incrementando la riga del valore di $p$**.

Riprendendo l'esempio precedente sulla colonna 43, se aumentiamo l'indice della riga sommandovi il valore del primo stesso ($9 + 11 = 20$) otteniamo:

$$(20 \times 60) + 43 = 1243 \quad \longrightarrow \quad 1243 \pmod{11} = 0$$

L'algoritmo non deve fare altro che far scorrere l'indice della riga sommandovi direttamente il valore di $p$ (`riga += p`) in un ciclo `while` elementare, fino al raggiungimento del limite superiore del segmento. In questo modo, l'eliminazione di tutti i multipli di un divisore su una colonna avviene alla velocità pura dell'hardware, azzerando divisioni, controlli complessi o salti caotici nella memoria.

⚠️ **Nota di Chiarimento sulla Gestione dei Segmenti Avanzati ($n_1, n_2, n_3...$):** L'esempio precedente descrive il calcolo lineare all'interno del **Segmento Zero** (ovvero partendo dall'origine). Quando l'algoritmo opera su segmenti avanzati oltre lo zero (che chiameremo livelli $n_1, n_2, n_3...$), la riga utilizzata nel ciclo diventa un indice locale che ricomincia da capo in ogni blocco.

Di conseguenza, per calcolare il valore numerico assoluto non basterà più applicare la formula `(Riga × 60) + Colonna`. Sarà invece necessario **adattare la riga al livello del segmento**, introducendo nel calcolo il valore di base del segmento stesso:

$$\text{Numero Reale} = \text{Base del Segmento} + (\text{Riga Locale} \times 60) + \text{Colonna}$$


_Dove $\text{Base del Segmento} = \text{Indice del Segmento} \times \text{SEGMENTO} \times 60$._

Questa scomposizione garantisce che ogni thread possa spogliare finestre numeriche a qualsiasi magnitudo mantenendo indici locali piccoli e leggeri, preservando la velocità hardware della CPU ed evitando l'uso di variabili troppo grandi all'interno dei cicli critici.

## Confronto Empirico

Il confronto è stato eseguito sullo stesso hardware, su tre diverse finestre W=100.000, 1.000.000, 10.000.000 a una altitudine di $10^{19}$

|  **Component**  |      **Detail**      |
| :-------------: | :------------------: |
|       CPU       |  AMD Ryzen 7 4800U   |
| Cores / Threads | 8 cores / 16 threads |
| Base frequency  |       1.80 GHz       |
|       RAM       |        32 GB         |
|       OS        |      Windows 11      |

In questo confronto ho utilizzato:
* **GC60_LRLN_x8.jl che lavora con il metodo a 8 colonne e LRLN**
* **GC60_M60_x16_LRN.jl, che lavora su 16 colonne e LRLN, ed è quello che presento qui**
* **primesieve 12.13, Copyright (C) 2010 - 2026 Kim Walisch  BSD 2-Clause License **

L'ultima riga rappresenta il superamento di $2^{64}$ e cioè $10^{20}$ dove il confronto si interrompe con primesieve che per motivi suoi logici ha impostato come $2^{64}$ il limite delle sua capacità


## Tabella 1: GC60 Julia (x16 vs x8) vs Primesieve

|**Magnitudo**|**Finestra (W)**|**Algoritmo**|**Divisori Geometrici**|**Tempo Fase B**|**Tempo Fase C**|**Tempo Totale**|**Primi Trovati**|
|---|---|---|---|---|---|---|---|
|**$10^{19}$**|100.000|**GC60_M60x16**|46.990|0,421 s|0,051 s|**0,495 s**|2.263|
|||GC60_LRLN_8x|46.990|0,773 s|0,077 s|0,905 s|2.263|
|||Primesieve|—|—|—|1,520 s|2.263|
|**$10^{19}$**|1.000.000|**GC60_M60x16**|353.851|0,453 s|0,071 s|**0,549 s**|23.069|
|||GC60_LRLN_8x|353.851|0,696 s|0,098 s|0,847 s|23.069|
|||Primesieve|—|—|—|1,534 s|23.069|
|**$10^{19}$**|10.000.000|**GC60_M60x16**|2.584.535|0,591 s|0,223 s|**0,834 s**|229.305|
|||GC60_LRLN_8x|2.584.535|0,799 s|0,208 s|1,061 s|229.305|
|||Primesieve|—|—|—|1,616 s|229.305|
|**$10^{20}$**|100.000|**GC60_M60x16**|49.714|1,951 s|0,050 s|**2,025 s**|2.115|
|||GC60_LRLN_8x|49.714|2,353 s|0,073 s|2,480 s|2.115|
|||Primesieve|—|—|—|**CRASH (Overflow)**|—|

Quello che possiamo vedere dalla tabella sopra è una netta differenza sul tempo finale del processo, x16 supera x8 di quasi mezzo secondo e primesieve di un secondo, mentre sia x16 che x8 superano primesieve. I numeri primi identificati all'interno della finestra sono identici per tutti e tre


### Tabella 2: GC60_M60x16_LRLN.cpp vs Primesieve

|**Magnitudo**|**Finestra (W)**|**Primi Rilevati**|**Tempo Totale GC-60**|**Tempo Primesieve**|**Rapporto di Velocità (Speedup)**|
|---|---|---|---|---|---|
|**10^17**|100.000|2.515|71 ms (0,071 s)|156 ms (0,156 s)|**GC-60 è ~2,2 volte più veloce**|
|**10^18**|100.000|2.398|108 ms (0,108 s)|486 ms (0,486 s)|**GC-60 è ~4,5 volte più veloce**|
|**10^19**|100.000|2.263|276 ms (0,276 s)|1.517 ms (1,517 s)|**GC-60 è ~5,5 volte più veloce**|

### Analisi e Spiegazione dei Risultati del Benchmark c++ vs primesieve

I dati emersi dal confronto prestazionale tra lo script C++ (`GC60_M60x16_LRLN.cpp`) e _Primesieve_ evidenziano alcune caratteristiche fondamentali del comportamento del modello GC-60 all'aumentare delle dimensioni numeriche gestite.

#### 1. Validazione Matematica e Accuratezza

Il primo elemento fondamentale è il perfetto allineamento del numero di primi rilevati all'interno della finestra (ad esempio, esattamente 2.263 primi individuati alla magnitudo $10^{19}$). Questa assoluta corrispondenza con i dati di _Primesieve_ conferma la correttezza matematica del filtro LRLN e del meccanismo di allineamento geometrico: il modello seleziona con precisione tutti i candidati reali senza subire perdite di informazioni o generare falsi positivi.

#### 2. L'Andamento dello Speedup (Efficienza di Scala)

Analizzando i tempi totali di esecuzione (Tabella 2), si nota che il vantaggio computazionale di GC-60 rispetto a _Primesieve_ non è costante, ma cresce in modo proporzionale alla Magnitudo:

- **A magnitudo $10^{17}$:** GC-60 completa l'elaborazione in 71 ms contro i 156 ms di _Primesieve_, con una velocità superiore di circa **2,2 volte**.
    
- **A magnitudo $10^{18}$:** Il tempo di GC-60 sale a 108 ms mentre _Primesieve_ richiede 486 ms, portando il vantaggio a circa **4,5 volte**.
    
- **A magnitudo $10^{19}$:** A fronte dei 1.517 ms richiesti da _Primesieve_, GC-60 risolve la finestra in soli 276 ms, dimostrandosi oltre **5,5 volte più rapido**.
    

Questo andamento indica che, nell'intervallo di computazione analizzato entro i 64 bit, l'algoritmo basato sul modello GC-60 mostra una progressione dei tempi più contenuta rispetto a un crivello segmentato tradizionale di riferimento.

### Guida alla Compilazione e all'Esecuzione

Per garantire la piena riproducibilità dei risultati presentati nei benchmark, di seguito vengono dettagliate le modalità di compilazione ed esecuzione dei programmi C++ e Julia nei diversi ambienti operativi.

#### 1. Compilazione del Codice C++ (GC60_M60x16_LRLN.cpp)

L'ottimizzazione del codice C++ richiede l'abilitazione delle istruzioni vettoriali AVX2 e del supporto al multi-threading tramite OpenMP.

**Ambiente Linux / WSL (Consigliato):** L'utilizzo di Linux (nativo o tramite Windows Subsystem for Linux - WSL) rappresenta la modalità più semplice e prestazionale. La compilazione si effettua direttamente da riga di comando con GCC:

Bash

```
g++ -O3 -march=native -fopenmp -mavx2 GC60_M60x16_LRLN.cpp -o GC60_M60x16_LRLN
```

Per lanciare l'eseguibile passando i parametri di Magnitudo e Finestra (ad esempio $10^{18}$ e $100.000$):

Bash

```
./GC60_M60x16_LRLN 1000000000000000000 100000
```




**Ambiente Windows (IDE):** Su Windows, la compilazione da riga di comando richiede una configurazione complessa dell'ambiente. Si consiglia pertanto l'uso di un IDE come Visual Studio o Code::Blocks, prestando attenzione alle seguenti impostazioni di progetto:

- Creare un nuovo progetto impostandolo come Applicazione Console (Console Application).
    
- Impostare la modalità di build su Release (evitando la modalità Debug, che comprometterebbe gravemente le prestazioni).
    
- Nelle proprietà del progetto (sezione C/C++ -> Ottimizzazione/Code Generation), abilitare il supporto alle istruzioni avanzate AVX2 (es. `/arch:AVX2` in MSVC) e il supporto OpenMP (es. `/openmp` in MSVC).
    

#### 2. Esecuzione dello Script Julia

A differenza del C++, il linguaggio Julia non richiede una fase di compilazione preventiva (è sufficiente averlo installato sul computer), ma la corretta allocazione delle risorse hardware è determinante per ottenere le prestazioni indicate nelle tabelle di confronto.

**Gestione dei Thread:** Per riprodurre fedelmente i tempi descritti nel benchmark, è indispensabile lanciare Julia allocando esplicitamente i 16 thread hardware. Da riga di comando o terminale, lo script va avviato con il flag `-t`:

Bash

```
julia -t 16 GC60_M60x16_LRLN.jl
```

_(In alternativa, è possibile impostare la variabile d'ambiente `JULIA_NUM_THREADS=16` all'interno del proprio sistema operativo prima di avviare l'interprete)._



### Prospettive di Sviluppo: Scalabilità a 32 Thread e Modulo 120

Per dimostrare l'elasticità e la scalabilità del modello GC-60, è utile presentare un'ipotesi di sviluppo teorico legata alle architetture hardware di nuova generazione. Attualmente, l'implementazione del mio script si ferma a 16 colonne poiché il sistema hardware utilizzato per i test dispone di 16 thread. È tuttavia interessante valutare cosa accadrebbe se il modello venisse eseguito su una macchina a 32 thread, estendendo il calcolo a un modulo 120.

#### 1. Perfetta Simmetria tra Matematica e Hardware

Il modulo 120 possiede **esattamente 32 residui coprimi** (ovvero numeri che non condividono divisori con 120, eccetto l'unità). Di conseguenza, la struttura del modello manterrebbe una corrispondenza speculare perfetta: **32 colonne matematiche assegnate a 32 thread hardware**. Questa configurazione elimina alla radice i tempi morti o le asimmetrie nella distribuzione del carico, garantendo un bilanciamento naturale e totale tra tutti i core operativi.

#### 2. Dilatazione dello Spazio Numerico (Colonne più Sfoltite)

Aumentando il modulo da 60 a 120, il "passo" geometrico all'interno di ogni singola colonna raddoppia:

- Nel modello attuale a modulo 60, i numeri interni a una colonna avanzano di 60 unità alla volta.

- Nel modello ipotetico a modulo 120, l'avanzamento sarebbe di 120 unità.


Ogni colonna diventa numericamente più "sottile". All'interno del singolo segmento di memoria (`SEGMENTO`), il ciclo che applica la traslazione dei divisori deve compiere la metà dei passi per marcare i multipli, riducendo il numero di accessi e di operazioni di scrittura in memoria per singola colonna.

#### 3. Scalabilità Lineare e Assenza di Conflitti

In coerenza con l'approccio divisore-centrico del progetto GC-60, in cui la finestra è un contenitore passivo, le 32 colonne rimangono totalmente indipendenti tra loro sia nella fase di setaccio che in quella di spogliatura. Ogni thread elabora la propria colonna senza necessità di scambiare dati o attendere i risultati degli altri core. Questa totale assenza di interdipendenza azzera i conflitti di memoria (_race conditions_), consentendo alle prestazioni di scalare in modo lineare con il raddoppio dei thread.

#### 4. Gestione della Fase di Bootstrap

L'unico incremento di lavoro si registra nella fase iniziale di inizializzazione: la tabella dei residui passa a 32 elementi e l'array degli inversi a 120 elementi. Trattandosi di strutture dati minuscole allocate una sola volta all'avvio del programma, il loro impatto sui tempi totali di esecuzione rimane infinitesimale e virtualmente invisibile nel computo complessivo.

#### Conclusioni sulla Scalabilità

Questa analisi teorica evidenzia come il modello GC-60 non sia un sistema rigido, ma un'architettura elastica. Il passaggio al modulo 120 dimostrerebbe che l'algoritmo può espandersi geometricamente per sfruttare appieno la potenza dei moderni processori multi-core, aumentando la granularità del calcolo senza introdurre alcuna complessità logica o strutturale.

### Il Ribaltamento del Paradigma dei Crivelli Classici

Un aspetto cruciale che merita di essere evidenziato, e che differenzia nettamente questo approccio dai metodi della letteratura classica, riguarda l'efficienza legata alle dimensioni della ruota (_wheel factorization_). Nei crivelli tradizionali, l'aumento delle dimensioni del modulo (passando ad esempio da 30 a 210, 2310, ecc.) comporta storicamente un progressivo calo delle prestazioni reali a causa dell'aumento della complessità dell'algoritmo e della perdita di efficienza nella gestione della memoria cache.

Questo paradigma viene modificato. Osservando sperimentalmente che il passaggio progressivo da 3, a 8, fino a 16 colonne, non indebolisce il sistema, ma al contrario gli conferisce maggiore forza e velocità. La ragione di questa inversione di tendenza risiede nel fatto che il nucleo strutturale rimane sempre ancorato alla stessa base matematica fondamentale dei primi primordiali ($2 \times 3 \times 5 = 30$), ma lo spazio numerico viene suddiviso e distribuito in modo puramente geometrico in base alla posizione dei residui. Invece di appesantire l'esecuzione all'interno di un unico flusso di calcolo, l'espansione del modello permette di segmentare i candidati in colonne sempre più sottili e tra loro totalmente indipendenti. Questo trasforma quello che nei crivelli classici rappresenta un noto punto critico in un potente fattore di accelerazione e parallelismo.
