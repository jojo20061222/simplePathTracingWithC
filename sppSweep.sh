#!/bin/bash
# spp sweep: decide how many samples per pixel the benchmark should use.
#
# 1. Renders two high-spp reference images with different seeds (both independent of the test
#    images). ref is the reference; the difference ref - ref2 measures the reference's own noise.
# 2. For spp = 1, 2, 4, ..., MAX_SPP: times every thread count in THREADS, RUNS times each,
#    and saves one image per spp: .txt (8-bit display) and .xyz (linear float64).
#    The image depends only on seed + spp, not on the thread count.
# 3. Run ./analyzeSpp.py <outdir> afterwards for RMSE / speedup / timing-variance tables.
#
# Settings (environment variables):
#   REF_SPP   spp of the reference image          (default 4096)
#   MAX_SPP   largest spp in the sweep            (default 128)
#   RUNS      repetitions per (spp, threads)      (default 3)
#   THREADS   thread counts to time               (default "1 <nproc>")
#   SEED      seed of the sweep images            (default 42)
#   REF_SEED  seed of the reference image         (default 999, must differ from SEED)
#   REF_SEED2 seed of the second reference image  (default 1000)
#   PIN_SINGLE  1 = pin single-thread runs to the fastest core with taskset (default 1).
#             This CPU mixes fast and slow cores, so an unpinned 1-thread run could land on either.
#   OUTDIR    output directory                    (default sppSweep)
#
# Example: MAX_SPP=64 RUNS=5 ./sppSweep.sh
set -euo pipefail

REF_SPP=${REF_SPP:-4096}
MAX_SPP=${MAX_SPP:-128}
RUNS=${RUNS:-3}
THREADS=${THREADS:-"1 $(nproc)"}
SEED=${SEED:-42}
REF_SEED=${REF_SEED:-999}
REF_SEED2=${REF_SEED2:-1000}
PIN_SINGLE=${PIN_SINGLE:-1}
OUTDIR=${OUTDIR:-sppSweep}
BIN=./bin/pathTracing
MAXT=$(nproc)

if [[ ! -x $BIN ]]; then
    echo "$BIN not found: build first (CC=gcc ./makeMakefile.sh example.c > Makefile && make all)"
    exit 1
fi
if [[ $SEED == "$REF_SEED" || $SEED == "$REF_SEED2" || $REF_SEED == "$REF_SEED2" ]]; then
    echo "SEED, REF_SEED and REF_SEED2 must all differ"
    exit 1
fi

# fastest core = highest max frequency (first one if several)
FASTCPU=$(lscpu -e=CPU,MAXMHZ | awk 'NR > 1 && $2 > best { best = $2; cpu = $1 } END { print cpu }')
run() { # run <threads> <args...>: pins single-thread runs to the fastest core
    local t=$1; shift
    if [[ $PIN_SINGLE == 1 && $t == 1 ]]; then
        taskset -c "$FASTCPU" "$BIN" --bench --threads 1 "$@"
    else
        "$BIN" --bench --threads "$t" "$@"
    fi
}
mkdir -p "$OUTDIR"

field() { sed -n "s/.*$1=\([^ ]*\).*/\1/p" <<< "$2"; }

# --- reference images (reused if they already exist with the same settings) ---
renderRef() { # renderRef <name> <seed>
    local name=$1 info="spp=$REF_SPP seed=$2"
    if [[ -f $OUTDIR/$name.txt && -f $OUTDIR/$name.xyz && -f $OUTDIR/$name.info &&
          $(cat "$OUTDIR/$name.info") == "$info" ]]; then
        echo "$name exists ($info), skipping"
        return
    fi
    echo "rendering $name ($info, $MAXT threads)..."
    local line
    line=$(run "$MAXT" --spp "$REF_SPP" --seed "$2" --out "$OUTDIR/$name.txt" \
               --out-xyz "$OUTDIR/$name.xyz" | grep '^BENCH')
    echo "$info" > "$OUTDIR/$name.info"
    echo "  $name done in $(field time_s "$line") s"
}
renderRef ref "$REF_SEED"
renderRef ref2 "$REF_SEED2"

# --- sweep ---
csv=$OUTDIR/timings.csv
{
    echo "# $(date -Iseconds) host=$(hostname) cpu=$(lscpu | sed -n 's/^Model name: *//p' | paste -sd+) cores=$MAXT seed=$SEED pin_single=$PIN_SINGLE fastcpu=$FASTCPU"
    echo "spp,threads,run,time_s,Msamples_per_s"
} > "$csv"

run "$MAXT" --spp 1 --seed "$SEED" > /dev/null # warm-up

spp=1
while (( spp <= MAX_SPP )); do
    saved=0
    for t in $THREADS; do
        for r in $(seq 1 "$RUNS"); do
            out=/dev/null; xyz=/dev/null
            # save the image once per spp (it is identical for every thread count and run)
            if (( saved == 0 )); then out=$OUTDIR/img_spp$spp.txt; xyz=$OUTDIR/img_spp$spp.xyz; saved=1; fi
            line=$(run "$t" --spp "$spp" --seed "$SEED" --out "$out" --out-xyz "$xyz" | grep '^BENCH')
            ts=$(field time_s "$line")
            echo "$spp,$t,$r,$ts,$(field Msamples_per_s "$line")" >> "$csv"
            printf "spp=%-5s threads=%-3s run=%s  %8.3f s\n" "$spp" "$t" "$r" "$ts"
        done
    done
    spp=$((spp * 2))
done

echo
echo "done. results in $OUTDIR/. next: .venv/bin/python analyzeSpp.py $OUTDIR"
