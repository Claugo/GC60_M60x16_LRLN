// ============================================================================
// PROGETTO: Modello GC-60
// SCRIPT  : GC60_M60x16_LRLN.cpp
// AUTORE  : Govi Claudio
// DATA    : Luglio 2026
//
// DESCRIZIONE:
// Questo programma implementa l'approccio "divisore-centrico" basato sul modello
// logico GC-60. Invece di eseguire una verifica diretta e globale
// sulla finestra di ricerca (considerata qui come un contenitore passivo), il
// codice esplora lo spazio numerico tramite la traslazione dei soli divisori noti
// e significativi, escludendo quelli informativamente irrilevanti.
// I numeri che sopravvivono a questa selezione rappresentano i candidati primi
// della finestra analizzata.
//
// L'algoritmo opera una scomposizione dello spazio numerico in 16 colonne distinte,
// sfruttando le proprietà dei residui coprimi del modulo 60.
//
// ARCHITETTURA E FASI DEL MOTORE PRINCIPALE:
// 1. BOOTSTRAP DINAMICO: Generazione e organizzazione iniziale dei divisori primi
//    di base necessari per avviare il setaccio.
// 2. FASE B (Setaccio Verticale & Filtro LRLN): Isolamento e calcolo dei divisori
//    geometrici stabili. Questa fase sfrutta un bilanciamento dinamico del carico
//    di lavoro sulle 16 colonne per massimizzare l'efficienza computazionale.
// 3. FASE C (Spogliatura della Finestra Passiva): Proiezione e traslazione dei
//    divisori isolati sulla finestra target (definita da Magnitudo e ampiezza W).
//    I numeri primi individuati vengono convertiti in offset rispetto alla Magnitudo
//    e salvati nel file di output "finestra_1.txt".
//
// REQUISITI E OTTIMIZZAZIONE TECNICA:
// - Parallelismo: Sviluppato per l'esecuzione multi-thread tramite direttive OpenMP.
// - Ottimizzazione: Predisposto per sfruttare le istruzioni vettoriali AVX2 e
//   l'allineamento geometrico puro a 64-bit senza l'impiego di numeri negativi.
//
// PARAMETRI DA LINEA DI COMANDO:
// - argv[1] (Opzionale): MAGNITUDO (Punto di partenza della finestra. Default: 10^19).
// - argv[2] (Opzionale): W (Ampiezza della finestra di ricerca. Default: 100.000).
//
// NB: Questo programma è pensato per arrivare al massimo a 64 bit. Questa è una scelta voluta per misurare la dinamica
//     di velocità nell'ambito dei sistemi a 64 bit
//
// COMPILAZIONE (g++):
// Linux Ubuntu: g++ -O3 -march=native -fopenmp -mavx2 GC60_M60x16_LRLN.cpp -o GC60_M60x16_LRLN
// ============================================================================

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <omp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cctype>
#include <algorithm>


typedef uint64_t u64;


// Funzione per convertire stringhe formattate (es. "10^19", "2^60", "100.000") in u64
u64 parse_numero(std::string str) {
    // 1. Rimuove eventuali punti o virgole usati come separatori delle migliaia
    str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
    str.erase(std::remove(str.begin(), str.end(), ','), str.end());
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());

    // 2. Controllo notazione a potenza (es. "10^19" o "2^60")
    size_t pos_hat = str.find('^');
    if (pos_hat != std::string::npos) {
        u64 base = std::stoull(str.substr(0, pos_hat));
        u64 exp = std::stoull(str.substr(pos_hat + 1));
        return static_cast<u64>(std::pow(base, exp));
    }

    // 3. Conversione di numeri interi standard
    return std::stoull(str);
}

// ==================================================
// CONFIGURAZIONE VARIABILI GLOBALI (IMPOSTAZIONI DI DEFAULT)
// ==================================================
u64 MAGNITUDO = 10000000000000000000ULL; // Default: 10^19
u64 W = 100000;                          // Default: 100.000
const int SEGMENTO = 262144;

const u64 RESTI[16] = {1, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 49, 53, 59};
int INV_60[60] = {0};

const std::vector<u64> bts_init = {7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 211, 223, 227, 229, 281, 283, 293, 307, 311, 313, 317};

// Inizializzazione dinamica della tabella degli inversi
void inizializza_inv60() {
    for (int i = 1; i < 60; ++i) {
        for (int j = 1; j < 60; ++j) {
            if ((i * j) % 60 == 1) {
                INV_60[i] = j;
                break;
            }
        }
    }
}

// Allineamento geometrico puro a 64-bit (Garantito senza numeri negativi)
inline u64 calcola_pos_colonna(u64 p, u64 n_cicli, int col_idx) {
    u64 base = n_cicli * (u64)SEGMENTO * 60ULL;
    u64 p2 = p * p;
    u64 target = (base > p2) ? base : p2;

    u64 q_min = (target + p - 1) / p;
    u64 q0 = (RESTI[col_idx] * (u64)INV_60[p % 60]) % 60ULL;

    u64 q_start;
    if (q_min <= q0) {
        q_start = q0;
    } else {
        u64 diff = q_min - q0;
        u64 steps = (diff + 59ULL) / 60ULL;
        q_start = q0 + steps * 60ULL;
    }

    u64 m = p * q_start;
    return (m / 60ULL) - (n_cicli * (u64)SEGMENTO);
}

// Funzioni di servizio per l'interfaccia utente
std::string formatta_numero(u64 n) {
    if (n == 0) return "0";
    std::string s = std::to_string(n);
    std::string res = "";
    int len = s.length();
    for (int i = 0; i < len; ++i) {
        res += s[i];
        if ((len - i - 1) > 0 && (len - i - 1) % 3 == 0) res += ".";
    }
    return res;
}

std::string formatta_tempo(double t) {
    int minuti = (int)(t / 60);
    double restante = t - (minuti * 60);
    int sec = (int)restante;
    int ms = (int)((restante - sec) * 1000);
    std::stringstream ss;
    ss << minuti << " min. " << sec << " sec. " << ms << " ms";
    return ss.str();
}

// ==================================================
// MOTORE PRINCIPALE
// ==================================================
// ==================================================
// MOTORE PRINCIPALE
// ==================================================
int main(int argc, char* argv[]) {
    // Lettura parametri da linea di comando con supporto a potenze e punti
    if (argc >= 2) {
        try {
            MAGNITUDO = parse_numero(argv[1]);
        } catch (...) {
            std::cerr << "Formato Magnitudo non valido. Uso il valore di default.\n";
        }
    }
    if (argc >= 3) {
        try {
            W = parse_numero(argv[2]);
        } catch (...) {
            std::cerr << "Formato Finestra (W) non valido. Uso il valore di default.\n";
        }
    }

    inizializza_inv60();
    auto t_start_totale = std::chrono::high_resolution_clock::now();

    std::cout << "==================================================\n";
    std::cout << " GC60_M60x16 + FILTRO LRLN (LOAD BALANCED)\n";
    std::cout << " Thread attivi hardware in uso: " << omp_get_max_threads() << "\n";
    std::cout << "==================================================\n";
    std::cout << "Magnitudo   : " << formatta_numero(MAGNITUDO) << "\n";
    std::cout << "Finestra (W): " << formatta_numero(W) << "\n\n";

    u64 radice_max = (u64)std::sqrt((double)MAGNITUDO);
    u64 radice_boot = (u64)std::sqrt((double)radice_max) + 1;
    u64 cicli_totali = (radice_max / (SEGMENTO * 60)) + 1;

    std::cout << "Radice max (Limite Setaccio B): " << formatta_numero(radice_max) << "\n";
    std::cout << "Cicli totali per colonna      : " << formatta_numero(cicli_totali) << "\n\n";

    // 1. BOOTSTRAP DINAMICO
    std::cout << "Generazione divisori di base in corso...\n";
    auto t_boot = std::chrono::high_resolution_clock::now();

    std::vector<u64> bts;
    std::vector<std::vector<uint8_t>> sieves_boot(16, std::vector<uint8_t>(SEGMENTO, 1));
    sieves_boot[0][0] = 0;

    for (int col = 0; col < 16; ++col) {
        for (u64 p : bts_init) {
            if (p > radice_boot) continue;
            u64 pos = calcola_pos_colonna(p, 0, col);
            while (pos < (u64)SEGMENTO) {
                sieves_boot[col][pos] = 0;
                pos += p;
            }
        }
    }
    for (int k = 0; k < SEGMENTO; ++k) {
        for (int col = 0; col < 16; ++col) {
            u64 val = 60ULL * k + RESTI[col];
            if (val > radice_boot) continue;
            if (sieves_boot[col][k] && val >= 7) bts.push_back(val);
        }
    }
    std::sort(bts.begin(), bts.end());
    bts.erase(std::unique(bts.begin(), bts.end()), bts.end());
    size_t n_primi_div = bts.size();

    auto t_boot_end = std::chrono::high_resolution_clock::now();
    std::cout << "Divisori di base pronti: " << formatta_numero(n_primi_div) << " primi\n";
    std::cout << "[Tempo bootstrap: " << formatta_tempo(std::chrono::duration<double>(t_boot_end - t_boot).count()) << "]\n\n";

    // 2. FASE B (SETACCIO VERTICALE CON BILANCIAMENTO DINAMICO)
    std::cout << "Avvio setacciamento verticale a 16 colonne con filtro LRLN...\n";
    auto t_sieve = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<u64>> divisori_threads(16);
    u64 limite_sbarramento = 2 * W;

    // Distribuzione dinamica del carico di lavoro sulle colonne
    #pragma omp parallel for schedule(dynamic, 1) num_threads(16)
    for (int col_idx = 0; col_idx < 16; ++col_idx)
    {
        uint8_t sieve[SEGMENTO];
        std::vector<u64> local_divs;
        local_divs.reserve(40000);

        std::vector<u64> pos_array(n_primi_div);
        for (size_t i = 0; i < n_primi_div; ++i) {
            pos_array[i] = calcola_pos_colonna(bts[i], 0, col_idx);
        }

        for (u64 idx = 0; idx < cicli_totali; ++idx) {
            std::memset(sieve, 1, SEGMENTO);
            if (idx == 0 && col_idx == 0) sieve[0] = 0;

            for (size_t i = 0; i < n_primi_div; ++i) {
                u64 p = bts[i];
                u64 pos = pos_array[i];
                while (pos < (u64)SEGMENTO) {
                    sieve[pos] = 0;
                    pos += p;
                }
                pos_array[i] = pos - (u64)SEGMENTO;
            }

            u64 seg_base = idx * (u64)SEGMENTO * 60ULL;

            int k_max = SEGMENTO;
            if (idx == cicli_totali - 1) {
                if (radice_max < seg_base + RESTI[col_idx]) {
                    k_max = 0;
                } else {
                    u64 max_k_val = (radice_max - seg_base - RESTI[col_idx]) / 60ULL;
                    if (max_k_val < (u64)SEGMENTO) {
                        k_max = (int)max_k_val + 1;
                    }
                }
            }

            for (int k = 0; k < k_max; ++k) {
                if (sieve[k]) {
                    u64 p = seg_base + 60ULL * k + RESTI[col_idx];

                    if (p <= limite_sbarramento) {
                        local_divs.push_back(p);
                    } else {
                        u64 R = MAGNITUDO % p;
                        if (R % 2 == 0 && (p - R) <= W) {
                            local_divs.push_back(p);
                        }
                    }
                }
            }
        }
        divisori_threads[col_idx] = std::move(local_divs);
    }

    std::vector<u64> divisori;
    for (int i = 0; i < 16; ++i) {
        divisori.insert(divisori.end(), divisori_threads[i].begin(), divisori_threads[i].end());
    }
    std::sort(divisori.begin(), divisori.end());
    divisori.erase(std::unique(divisori.begin(), divisori.end()), divisori.end());

    auto t_sieve_end = std::chrono::high_resolution_clock::now();
    std::cout << " Divisori geometrici stabili isolati: " << formatta_numero(divisori.size()) << "\n";
    std::cout << " [Tempo Fase B (Setaccio + LRLN): " << formatta_tempo(std::chrono::duration<double>(t_sieve_end - t_sieve).count()) << "] \n\n";

    // 3. FASE C (SPOGLIATURA COLLASSATA PER MASSIMO PARALLELISMO)
    std::cout << "Fase C: spogliatura della finestra passiva a " << formatta_numero(MAGNITUDO) << "...\n";
    auto t_start_C = std::chrono::high_resolution_clock::now();

    u64 seg_inizio = MAGNITUDO / (SEGMENTO * 60);
    u64 seg_fine = (MAGNITUDO + W) / (SEGMENTO * 60);

    std::vector<u64> primi_offset;

    #pragma omp parallel num_threads(16)
    {
        uint8_t sieve_finestra[SEGMENTO];
        std::vector<u64> local_primi;
        local_primi.reserve(2000);

        // Collassiamo i cicli (Colonne x Segmenti) distribuendoli dinamicamente
        #pragma omp for collapse(2) schedule(dynamic, 1)
        for (int col_idx = 0; col_idx < 16; ++col_idx) {
            for (u64 idx = seg_inizio; idx <= seg_fine; ++idx) {
                std::memset(sieve_finestra, 1, SEGMENTO);

                for (u64 p : divisori) {
                    u64 pos = calcola_pos_colonna(p, idx, col_idx);
                    while (pos < (u64)SEGMENTO) {
                        sieve_finestra[pos] = 0;
                        pos += p;
                    }
                }

                u64 base_segmento = idx * (u64)SEGMENTO * 60ULL;
                for (int k = 0; k < SEGMENTO; ++k) {
                    if (sieve_finestra[k]) {
                        u64 n = base_segmento + 60ULL * k + RESTI[col_idx];
                        if (n >= MAGNITUDO && n <= MAGNITUDO + W) {
                            local_primi.push_back(n - MAGNITUDO);
                        }
                    }
                }
            }
        }

        #pragma omp critical
        primi_offset.insert(primi_offset.end(), local_primi.begin(), local_primi.end());
    }

    std::sort(primi_offset.begin(), primi_offset.end());
    auto t_end_C = std::chrono::high_resolution_clock::now();
    std::cout << " Primi reali trovati nella finestra: " << formatta_numero(primi_offset.size()) << "\n";
    std::cout << " [Tempo Fase C: " << formatta_tempo(std::chrono::duration<double>(t_end_C - t_start_C).count()) << "]\n\n";

    auto t_end_totale = std::chrono::high_resolution_clock::now();
    std::cout << " TEMPO REALE TOTALE DI ESECUZIONE: " << formatta_tempo(std::chrono::duration<double>(t_end_totale - t_start_totale).count()) << "\n";
    std::cout << "==================================================\n";

    std::ofstream f("finestra_1.txt");
    if (f.is_open()) {
        f << "# A=" << formatta_numero(MAGNITUDO) << " W=" << W << "\n";
        for (u64 offset : primi_offset) f << offset << "\n";
        f.close();
        std::cout << " Lista degli offset salvata nel file: finestra_1.txt\n";
    }

    return 0;
}
