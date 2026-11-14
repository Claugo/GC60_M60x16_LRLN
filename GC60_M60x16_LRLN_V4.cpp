// g++ -O3 -fopenmp -march=native GC60_M60x16_LRLN_V4.cpp -o GC60_M60x16_LRLN_V4
// ============================================================================
// PROGETTO : Modello GC-60 / GC-120
// SCRIPT   : M60x16_LRLN_V4_SEG.cpp
// AUTORE   : Govi Claudio
// VERSION  : V4.0 (Fase B Bit-Sieve + Fase C Segmentata 1-Bit Parallela + 64/128-bit)
// DATA     : Novembre 2026
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
#include <cstdint>

typedef uint64_t u64;
typedef unsigned __int128 u128;

// Parametri Fase B (Setaccio sulle 16 colonne mod 60)
const u64 SEGMENTO = 262144;
const size_t WORDS_PER_SEG = SEGMENTO / 64; // 32 KB per thread

// Parametri Fase C (Finestra segmentata a 1-bit in cache L1)
const u64 BLOCK_SHIFT = 18;
const u64 BLOCK_CELLS = 1ULL << BLOCK_SHIFT;      // 262.144 celle dispari = 32 KB a 1-bit
const u64 BLOCK_MASK  = BLOCK_CELLS - 1;
const size_t WORDS_PER_BLOCK = BLOCK_CELLS / 64;  // 4.096 parole uint64_t
const u64 SPLIT_P = 16ULL * BLOCK_CELLS;          // 4.194.304: soglia divisori piccoli/grandi

const u64 RESTI[16] = {1, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 49, 53, 59};
int INV_60[60] = {0};

const std::vector<u64> bts_init = {
    7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89,
    97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179,
    211, 223, 227, 229, 281, 283, 293, 307, 311, 313, 317
};

struct DivisorePiccolo {
    u64 p;
    u64 idx_start;
};

u128 pow_u128(u128 base, u64 exp) {
    u128 res = 1;
    while (exp > 0) {
        if (exp & 1) res *= base;
        base *= base;
        exp >>= 1;
    }
    return res;
}

u128 parse_raw_u128(const std::string& s) {
    u128 res = 0;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            res = res * 10 + (u128)(c - '0');
        }
    }
    return res;
}

u128 parse_arg(const std::string& s) {
    size_t pos_caret = s.find('^');
    if (pos_caret != std::string::npos) {
        u128 base = parse_raw_u128(s.substr(0, pos_caret));
        u64 exp = std::stoull(s.substr(pos_caret + 1));
        return pow_u128(base, exp);
    }

    size_t pos_e = s.find_first_of("eE");
    if (pos_e != std::string::npos) {
        std::string mantissa = s.substr(0, pos_e);
        int exp = std::stoi(s.substr(pos_e + 1));
        int decimali = 0;
        std::string cifre = "";
        for (size_t i = 0; i < mantissa.size(); ++i) {
            if (mantissa[i] == '.') {
                decimali = (int)(mantissa.size() - i - 1);
            } else {
                cifre += mantissa[i];
            }
        }
        u128 val = parse_raw_u128(cifre);
        int exp_netto = exp - decimali;
        if (exp_netto >= 0) {
            val *= pow_u128(10, (u64)exp_netto);
        } else {
            val /= pow_u128(10, (u64)(-exp_netto));
        }
        return val;
    }

    return parse_raw_u128(s);
}

u64 isqrt_u128(u128 n) {
    if (n == 0) return 0;
    u64 low = 1, high = UINT64_MAX, ans = 1;
    while (low <= high) {
        u64 mid = low + (high - low) / 2;
        u128 sq = (u128)mid * mid;
        if (sq == n) return mid;
        if (sq < n) {
            ans = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return ans;
}

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

template <typename T>
inline u64 calcola_pos_colonna(u64 p, u64 n_cicli, int col_idx) {
    T base = (T)n_cicli * SEGMENTO * 60ULL;
    T p2 = (T)p * p;
    T target = (base > p2) ? base : p2;

    T q_min = (target + p - 1) / p;
    T q0 = (RESTI[col_idx] * (u64)INV_60[p % 60]) % 60ULL;

    T q_start = (q_min <= q0) ? q0 : q0 + ((q_min - q0 + 59ULL) / 60ULL) * 60ULL;
    T m = (T)p * q_start;
    return (u64)((m / 60ULL) - ((T)n_cicli * SEGMENTO));
}

// Riduzione hardware veloce per 128-bit (sfrutta DIVQ nativo x86-64)
inline u64 fast_rem_mag_128(u64 mag_h, u64 mag_l, u64 p) {
    u64 rem_2_64 = (0ULL - p) % p;
    u128 val = (u128)mag_h * rem_2_64 + mag_l;
#if defined(__x86_64__) || defined(_M_X64)
    u64 hi = (u64)(val >> 64);
    u64 lo = (u64)val;
    u64 rem;
    __asm__("divq %[div]" : "+a"(lo), "=d"(rem) : "d"(hi), [div]"r"(p));
    return rem;
#else
    return (u64)(val % p);
#endif
}

std::string formatta_numero(u128 n) {
    if (n == 0) return "0";
    std::string s = "";
    while (n > 0) {
        s += (char)('0' + (int)(n % 10));
        n /= 10;
    }
    std::reverse(s.begin(), s.end());
    std::string res = "";
    int len = (int)s.length();
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

template <typename T>
void esegui_programma(T MAGNITUDO, u64 W) {
    u64 W_DISPARI = W / 2;
    size_t num_blocks = (W_DISPARI == 0) ? 0 : (size_t)((W_DISPARI + BLOCK_CELLS - 1) / BLOCK_CELLS);

    inizializza_inv60();
    auto t_start_totale = std::chrono::high_resolution_clock::now();
    int num_threads = omp_get_max_threads();

    const u64 MAG_H = (sizeof(T) == 16) ? (u64)((u128)MAGNITUDO >> 64) : 0ULL;
    const u64 MAG_L = (u64)MAGNITUDO;
    const u64 mag_odd = (u64)(MAGNITUDO & 1);
    const u64 mag_mod3 = (u64)(MAGNITUDO % 3);
    const u64 mag_mod5 = (u64)(MAGNITUDO % 5);

    std::cout << "==================================================\n";
    std::cout << " GC60 + SETACCIO SEGMENTATO 1-BIT (V4.0)\n";
    std::cout << " Thread attivi hardware in uso: " << num_threads << "\n";
    std::cout << " Configurazione aritmetica    : "
              << (sizeof(T) == 8 ? "64-bit (uint64_t)" : "128-bit (__uint128_t)") << "\n";
    std::cout << "==================================================\n";
    std::cout << "Magnitudo         : " << formatta_numero(MAGNITUDO) << "\n";
    std::cout << "Finestra (W)      : " << formatta_numero(W) << "\n";
    std::cout << "Dim. Compressa W/2: " << formatta_numero(W_DISPARI)
              << " celle (" << num_blocks << " blocchi da 32 KB)\n\n";

    u64 radice_max = isqrt_u128(MAGNITUDO);
    u64 radice_boot = isqrt_u128(radice_max) + 1;
    u64 cicli_totali = (radice_max / (SEGMENTO * 60)) + 1;

    // 1. BOOTSTRAP DINAMICO
    auto t_boot = std::chrono::high_resolution_clock::now();
    std::vector<u64> bts;
    std::vector<std::vector<uint8_t>> sieves_boot(16, std::vector<uint8_t>(SEGMENTO, 1));
    sieves_boot[0][0] = 0;

    for (int col = 0; col < 16; ++col) {
        for (u64 p : bts_init) {
            if (p > radice_boot) continue;
            u64 pos = calcola_pos_colonna<T>(p, 0, col);
            while (pos < SEGMENTO) {
                sieves_boot[col][pos] = 0;
                pos += p;
            }
        }
    }
    for (size_t k = 0; k < SEGMENTO; ++k) {
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

    // 2. FASE B: SETACCIO + SMISTAMENTO DIRETTO NEI SECCHIELLI DEI BLOCCHI
    std::cout << "Avvio Fase B: Setaccio + Smistamento nei blocchi...\n";
    auto t_sieve = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<DivisorePiccolo>> small_locali(num_threads);
    std::vector<std::vector<std::vector<uint32_t>>> thread_buckets(
        num_threads, std::vector<std::vector<uint32_t>>(num_blocks)
    );

    size_t stima_bucket = (num_blocks > 0)
        ? (size_t)((0.40 * (double)W_DISPARI) / ((double)num_blocks * num_threads)) + 64
        : 0;

    for (int t = 0; t < num_threads; ++t) {
        small_locali[t].reserve(300000 / num_threads + 256);
        for (size_t b = 0; b < num_blocks; ++b) {
            thread_buckets[t][b].reserve(stima_bucket);
        }
    }

    u64 divisori_utili_count = 0;

    #pragma omp parallel num_threads(num_threads) reduction(+:divisori_utili_count)
    {
        int t_id = omp_get_thread_num();
        u64 sieve[WORDS_PER_SEG];
        std::vector<u64> pos_array(n_primi_div);
        auto& small_vec = small_locali[t_id];
        auto& buckets = thread_buckets[t_id];

        #pragma omp for schedule(dynamic, 1)
        for (int col_idx = 0; col_idx < 16; ++col_idx) {
            for (size_t i = 0; i < n_primi_div; ++i) {
                pos_array[i] = calcola_pos_colonna<T>(bts[i], 0, col_idx);
            }

            for (u64 idx = 0; idx < cicli_totali; ++idx) {
                memset(sieve, 0xFF, sizeof(sieve));
                if (idx == 0 && col_idx == 0) sieve[0] &= ~1ULL;

                for (size_t i = 0; i < n_primi_div; ++i) {
                    u64 p = bts[i];
                    u64 pos = pos_array[i];
                    while (pos < SEGMENTO) {
                        sieve[pos >> 6] &= ~(1ULL << (pos & 63));
                        pos += p;
                    }
                    pos_array[i] = pos - SEGMENTO;
                }

                T seg_base = (T)idx * SEGMENTO * 60ULL;
                const u64 r_col = RESTI[col_idx];

                for (size_t w = 0; w < WORDS_PER_SEG; ++w) {
                    u64 word = sieve[w];
                    while (word != 0) {
                        u64 bit = (u64)__builtin_ctzll(word);
                        size_t k = (w << 6) | bit;
                        word &= word - 1;

                        T p_val = seg_base + 60ULL * k + r_col;
                        if (p_val < 7) continue;
                        if (p_val > radice_max) break;
                        u64 p = (u64)p_val;

                        u64 R = (sizeof(T) == 8)
                            ? (MAG_L % p)
                            : fast_rem_mag_128(MAG_H, MAG_L, p);

                        u64 pos_ingresso = (R == 0) ? 0 : (p - R);
                        if (((mag_odd ^ pos_ingresso) & 1) == 0) {
                            pos_ingresso += p;
                        }

                        if (pos_ingresso <= W) {
                            u64 idx_0 = (pos_ingresso - 1) / 2;
                            if (idx_0 < W_DISPARI) {
                                divisori_utili_count++;
                                if (p < SPLIT_P) {
                                    small_vec.push_back({p, idx_0});
                                } else {
                                    for (u64 idx_w = idx_0; idx_w < W_DISPARI; idx_w += p) {
                                        size_t b = (size_t)(idx_w >> BLOCK_SHIFT);
                                        uint32_t off = (uint32_t)(idx_w & BLOCK_MASK);
                                        buckets[b].push_back(off);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<DivisorePiccolo> small_divs;
    size_t tot_small = 0;
    for (int t = 0; t < num_threads; ++t) tot_small += small_locali[t].size();
    small_divs.reserve(tot_small);
    for (int t = 0; t < num_threads; ++t) {
        small_divs.insert(small_divs.end(), small_locali[t].begin(), small_locali[t].end());
    }

    auto t_sieve_end = std::chrono::high_resolution_clock::now();
    std::cout << " Divisori utili isolati: " << formatta_numero(divisori_utili_count) << "\n";
    std::cout << " [Tempo Fase B: "
              << formatta_tempo(std::chrono::duration<double>(t_sieve_end - t_sieve).count()) << "]\n\n";

    // 3. FASE C: SPOGLIATURA PARALLELA A 1-BIT IN CACHE L1 (BLOCCHI DA 32 KB)
    std::cout << "Fase C: spogliatura parallela a 1-bit in cache L1...\n";
    auto t_start_C = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<u64>> primi_blocchi(num_blocks);

    #pragma omp parallel num_threads(num_threads)
    {
        u64 bitset[WORDS_PER_BLOCK]; // 32 KB: rimane interamente nella cache L1 del core

        #pragma omp for schedule(dynamic, 1)
        for (size_t b = 0; b < num_blocks; ++b) {
            u64 b_start = (u64)b << BLOCK_SHIFT;
            u64 b_len = std::min<u64>(BLOCK_CELLS, W_DISPARI - b_start);

            memset(bitset, 0xFF, sizeof(bitset));
            if (b_len < BLOCK_CELLS) {
                size_t full_words = (size_t)(b_len >> 6);
                size_t rem_bits = (size_t)(b_len & 63);
                if (rem_bits > 0) {
                    bitset[full_words] = (1ULL << rem_bits) - 1ULL;
                    for (size_t w = full_words + 1; w < WORDS_PER_BLOCK; ++w) bitset[w] = 0ULL;
                } else {
                    for (size_t w = full_words; w < WORDS_PER_BLOCK; ++w) bitset[w] = 0ULL;
                }
            }

            // 1. Applicazione divisori piccoli (< SPLIT_P) nel blocco da 32 KB
            for (const auto& sd : small_divs) {
                u64 p = sd.p;
                u64 idx0 = sd.idx_start;
                u64 pos;
                if (idx0 >= b_start) {
                    pos = idx0 - b_start;
                } else {
                    u64 rem = (b_start - idx0) % p;
                    pos = (rem == 0) ? 0 : (p - rem);
                }
                while (pos < b_len) {
                    bitset[pos >> 6] &= ~(1ULL << (pos & 63));
                    pos += p;
                }
            }

            // 2. Applicazione colpi diretti dei divisori grandi (>= SPLIT_P) in L1
            for (int t = 0; t < num_threads; ++t) {
                const auto& vec = thread_buckets[t][b];
                for (uint32_t off : vec) {
                    bitset[off >> 6] &= ~(1ULL << (off & 63));
                }
            }

            // 3. Estrazione istantanea dei candidati primi superstiti in ordine crescente
            auto& out_b = primi_blocchi[b];
            out_b.reserve((size_t)(b_len / 16) + 32);

            for (size_t w = 0; w < WORDS_PER_BLOCK; ++w) {
                u64 word = bitset[w];
                while (word != 0) {
                    u64 bit = (u64)__builtin_ctzll(word);
                    word &= word - 1;
                    u64 global_i = b_start + ((u64)w << 6) + bit;
                    u64 offset_reale = (global_i << 1) + 1;

                    if ((mag_mod3 + offset_reale) % 3 != 0 && (mag_mod5 + offset_reale) % 5 != 0) {
                        out_b.push_back(offset_reale);
                    }
                }
            }
        }
    }

    size_t tot_primi = 0;
    for (size_t b = 0; b < num_blocks; ++b) tot_primi += primi_blocchi[b].size();

    std::vector<u64> primi_offset;
    primi_offset.reserve(tot_primi);
    for (size_t b = 0; b < num_blocks; ++b) {
        primi_offset.insert(primi_offset.end(), primi_blocchi[b].begin(), primi_blocchi[b].end());
    }

    auto t_end_C = std::chrono::high_resolution_clock::now();
    std::cout << " Primi reali trovati nella finestra: " << formatta_numero(primi_offset.size()) << "\n";
    std::cout << " [Tempo Fase C: " << formatta_tempo(std::chrono::duration<double>(t_end_C - t_start_C).count()) << "]\n\n";

    auto t_end_totale = std::chrono::high_resolution_clock::now();
    std::cout << " TEMPO REALE TOTALE DI ESECUZIONE: "
              << formatta_tempo(std::chrono::duration<double>(t_end_totale - t_start_totale).count()) << "\n";
    std::cout << "==================================================\n";

    std::ofstream f("finestra_1.txt");
    if (f.is_open()) {
        f << "# A=" << formatta_numero(MAGNITUDO) << " W=" << W << "\n";
        for (u64 offset : primi_offset) f << offset << "\n";
        f.close();
        std::cout << " Lista degli offset salvata nel file: finestra_1.txt\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        std::cout << "Uso: " << argv[0] << " [MAGNITUDO] [FINESTRA_W]\n";
        std::cout << "Esempi supportati:\n";
        std::cout << "  " << argv[0] << " 10^19 10^7\n";
        std::cout << "  " << argv[0] << " 10^19 10^8\n";
        std::cout << "  " << argv[0] << " 10^19 10^9\n";
        std::cout << "  " << argv[0] << " 10^20 10^7\n";
        return 0;
    }

    u128 MAGNITUDO = 10000000000000000000ULL; // 10^19
    u64 W = 10000000ULL;                     // 10^7

    if (argc > 1) {
        MAGNITUDO = parse_arg(argv[1]);
    }
    if (argc > 2) {
        W = (u64)parse_arg(argv[2]);
    }

    const u128 MAX_64 = (u128)UINT64_MAX;
    u128 margine_sicurezza = (u128)W + 64 * (u128)isqrt_u128(MAGNITUDO);

    if (MAGNITUDO <= MAX_64 && margine_sicurezza <= MAX_64 - MAGNITUDO) {
        esegui_programma<u64>((u64)MAGNITUDO, W);
    } else {
        esegui_programma<u128>(MAGNITUDO, W);
    }

    return 0;
}