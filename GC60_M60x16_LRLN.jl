# julia -t 16 GC60_M60x16_LRLN.jl
# STEP 2: LRLN Geometrico su Architettura x16 (Index Sliding & Modulo Hardware)
# Autore: Govi Claudio - 2026
print("\033c")
using Base.Threads
using Printf

# =====================================================================
# CONFIGURAZIONE DEI PARAMETRI GLOBALI
# =====================================================================
const MAGNITUDO = Int128(10)^19  
const W         = 100000        
const SEGMENTO  = 262144         

# I 16 residui coprimi del modulo 60
const RESTI     = Int64[1, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 49, 53, 59]

# Pre-calcolo delle parti alta e bassa di MAGNITUDO (10^20) per riduzione hardware rapida
const MAG_H     = UInt64(MAGNITUDO >> 64)
const MAG_L     = UInt64(MAGNITUDO & 0xffffffffffffffff)

# Pre-calcolo dell'inverso modulare mod 60 per i coprimi
const INV_60 = zeros(Int, 60)
for i in 1:59
    if gcd(i, 60) == 1
        for j in 1:59
            if (i * j) % 60 == 1
                INV_60[i] = j
                break
            end
        end
    end
end

# Primi di base per il bootstrap iniziale
const bts_init = Int64[7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89,
    97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 211, 223, 227, 229, 281, 283, 293, 307, 311, 313, 317]

# =====================================================================
# FUNZIONI UTILI
# =====================================================================
function formatta_numero(n::Integer)
    s = string(n); len = length(s); res = ""
    for i in 1:len
        res *= string(s[i])
        if (len - i) > 0 && (len - i) % 3 == 0; res *= "."; end
    end
    return res
end

function formatta_tempo(t::Float64)
    minuti = floor(Int, t / 60)
    restante = t - (minuti * 60)
    str_sec = @sprintf("%.3f", restante)
    parti = split(str_sec, ".")
    return "$(minuti) min. $(parti[1]) sec. $(parti[2]) ms"
end

# Calcolo algebrico della riga di partenza
function calcola_pos_colonna(p::Int64, n_cicli::Int, col_idx::Int)
    base = Int128(n_cicli) * SEGMENTO * 60
    target = max(base, Int128(p) * p)
    
    q_min = div(target + p - 1, p)
    q0 = (RESTI[col_idx] * INV_60[p % 60]) % 60
    
    diff = q_min - q0
    if diff <= 0
        q_start = q0
    else
        steps = div(diff + 59, 60)
        q_start = q0 + steps * 60
    end
    
    m = Int128(p) * q_start
    pos = Int64(div(m, 60) - (Int128(n_cicli) * SEGMENTO))
    return pos
end

# Calcolo ultra-veloce di MAGNITUDO % p usando la divisione hardware nativa 128/64 bit
@inline function fast_rem_mag(p::Int64)
    p_u = UInt64(p)
    rem_2_64 = UInt64((UInt128(1) << 64) % p_u)
    val = UInt128(MAG_H) * rem_2_64 + MAG_L
    return Int64(val % p_u)
end

# =====================================================================
# BOOTSTRAP DINAMICO (PREPARA I DIVISORI FINO A SQRT(RADICE_MAX))
# =====================================================================
function bootstrap_divisori(radice_max::Int64)
    radice_boot = isqrt(radice_max) + 1
    bts_sieve = filter(p -> p <= radice_boot, bts_init)
    
    sieves = [BitVector(undef, SEGMENTO) for _ in 1:16]
    for col_idx in 1:16
        fill!(sieves[col_idx], true)
        if col_idx == 1; sieves[1][1] = false; end
        for p in bts_sieve
            pos = calcola_pos_colonna(p, 0, col_idx)
            while pos < SEGMENTO
                sieves[col_idx][pos + 1] = false
                pos += p
            end
        end
    end
    
    bts = Int64[]
    for k in 0:(SEGMENTO - 1)
        for col_idx in 1:16
            val = 60 * k + RESTI[col_idx]
            if val > radice_max; continue; end
            if sieves[col_idx][k + 1]
                if val >= 7
                    push!(bts, val)
                end
            end
        end
    end
    return unique!(sort!(bts))
end

# Cernita dei divisori utili per la finestra
function e_divisore_utile_60(p::Int64, a_start::Int128, a_fine::Int128)
    k_start = div(a_start + p - 1, p)
    k_end = div(a_fine, p)
    if k_start > k_end
        return false
    end
    if (k_end - k_start) >= 60
        return true
    end
    for k in k_start:k_end
        if k % 2 != 0 && k % 3 != 0 && k % 5 != 0
            return true
        end
    end
    return false
end

# =====================================================================
# CONTROLLO PRINCIPALE
# =====================================================================
function main()
    t_start_totale = time()

    println("="^50)
    println(" GC60_M60x16 + FILTRO LRLN")
    println(" Thread attivi hardware in uso: $(Threads.nthreads())")
    println("="^50)
    println("Magnitudo   : $(formatta_numero(MAGNITUDO))")
    println("Finestra (W): $(formatta_numero(W))")

    radice_max   = Int64(isqrt(MAGNITUDO))
    cicli_totali = Int(div(radice_max, SEGMENTO * 60) + 1)

    println("Radice max (Limite Setaccio B): $(formatta_numero(radice_max))")
    println("Cicli totali per colonna      : $(formatta_numero(cicli_totali))\n")

    # 1. Generazione dei divisori di base (fino a sqrt(10^10) = 100.000)
    t_boot = time()
    println("Generazione divisori di base in corso...")
    
    # CORREZIONE BUG: bootstrap limitato alla radice quadrata di radice_max!
    bts = bootstrap_divisori(Int64(isqrt(radice_max))) 
    n_primi_div = length(bts)
    println("Divisori di base pronti: $(formatta_numero(n_primi_div)) primi")
    println("[Tempo bootstrap: $(formatta_tempo(time() - t_boot))]\n")

    # 2. Setacciamento verticale e Filtro LRLN in corsa (1 Thread x Colonna)
    println("Avvio setacciamento verticale a 16 colonne con filtro LRLN...")
    t_sieve = time()
    
    limite_sbarramento = 2 * W
    divisori_threads = [Int64[] for _ in 1:16]

    Threads.@threads for col_idx in 1:16
        sieve = BitVector(undef, SEGMENTO)
        local_divs = Int64[]

        # Inizializzazione delle posizioni al segmento 0
        pos_array = Int64[calcola_pos_colonna(p, 0, col_idx) for p in bts]

        for idx in 0:(cicli_totali - 1)
            fill!(sieve, true)
            
            if idx == 0 && col_idx == 1
                sieve[1] = false
            end

            # Spogliatura super-veloce con soli 10.066 primi di sfoltimento
            for i in 1:n_primi_div
                p = bts[i]
                pos = pos_array[i]
                
                @inbounds while pos < SEGMENTO
                    sieve[pos + 1] = false
                    pos += p
                end
                pos_array[i] = pos - SEGMENTO
            end

            # FILTRO LRLN IN CORSA CON RIDUZIONE HARDWARE RAPIDA
            seg_base = Int128(idx) * SEGMENTO * 60
            for k in 0:(SEGMENTO - 1)
                @inbounds if sieve[k + 1]
                    p = Int64(seg_base + 60 * k + RESTI[col_idx])
                    if p < 7; continue; end
                    if p > radice_max; break; end

                    if p <= limite_sbarramento
                        push!(local_divs, p)
                    else
                        # Usiamo la nuova riduzione ultra-rapida a 64 bit!
                        R = fast_rem_mag(p)
                        if R % 2 == 0 && (p - R) <= W
                            push!(local_divs, p)
                        end
                    end
                end
            end
        end
        divisori_threads[col_idx] = local_divs
    end

    divisori = Int64[]
    for t in 1:16
        append!(divisori, divisori_threads[t])
    end
    unique!(sort!(divisori))

    tempo_fase_b = time() - t_sieve
    println(" Divisori geometrici stabili isolati: $(formatta_numero(length(divisori)))")
    println(" [Tempo Fase B (Setaccio + LRLN): $(formatta_tempo(tempo_fase_b))]\n")

    # -----------------------------------------------------------------
    # FASE C: APPLICAZIONE FINESTRA SULLA MAGNITUDO (10^20)
    # -----------------------------------------------------------------
    t_start_C = time()
    println("Fase C: spogliatura della finestra passiva a $MAGNITUDO...")

    seg_inizio = Int(div(MAGNITUDO, SEGMENTO * 60))
    seg_fine   = Int(div(MAGNITUDO + W, SEGMENTO * 60))

    primi_offset = Int64[]
    lk = ReentrantLock()

    Threads.@threads for col_idx in 1:16
        sieve_finestra = BitVector(undef, SEGMENTO)
        local_primi = Int64[]

        for idx in seg_inizio:seg_fine
            fill!(sieve_finestra, true)

            for p in divisori
                pos = calcola_pos_colonna(p, idx, col_idx)
                @inbounds while pos < SEGMENTO
                    sieve_finestra[pos + 1] = false
                    pos += p
                end
            end

            base_segmento = Int128(idx) * SEGMENTO * 60
            for k in 0:(SEGMENTO - 1)
                @inbounds if sieve_finestra[k + 1]
                    n = base_segmento + 60 * k + RESTI[col_idx]
                    if MAGNITUDO <= n <= MAGNITUDO + W
                        push!(local_primi, Int64(n - MAGNITUDO))
                    end
                end
            end
        end

        lock(lk) do
            append!(primi_offset, local_primi)
        end
    end
    sort!(primi_offset)

    println(" Primi reali trovati nella finestra: $(formatta_numero(length(primi_offset)))")
    println(" [Tempo Fase C: $(formatta_tempo(time() - t_start_C))]\n")

    # -----------------------------------------------------------------
    # CONCLUSIONE E SALVATAGGIO
    # -----------------------------------------------------------------
    t_end_totale = time()
    println(" TEMPO REALE TOTALE DI ESECUZIONE: $(formatta_tempo(t_end_totale - t_start_totale))")
    println("==============================\n")

    nome_file = "finestra_1.txt"
    open(nome_file, "w") do f
        println(f, "# A=$(MAGNITUDO) W=$(W)")
        for offset in primi_offset
            println(f, offset)
        end
    end
    println(" Lista degli offset salvata nel file: $nome_file")
end

main();