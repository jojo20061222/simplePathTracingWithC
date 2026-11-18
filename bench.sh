#!/bin/bash
# CPU baseline benchmark: fixed workload, several thread counts, repeated runs.
# Usage: ./bench.sh [spp=64] [runs=5] [seed=42]   -> prints CSV, saves benchResults.csv
spp=${1:-64}; runs=${2:-5}; seed=${3:-42}
bin=./bin/pathTracing
max=$(nproc)
out=benchResults.csv
echo "# $(date -Iseconds) host=$(hostname) cpu=$(lscpu | sed -n 's/^Model name: *//p' | paste -sd+) cores=$max spp=$spp seed=$seed" > $out
echo "threads,run,time_s,Msamples_per_s" >> $out

threads=(1); t=2; while [ $t -lt $max ]; do threads+=($t); t=$((t*2)); done; threads+=($max)

$bin --bench --spp 1 --threads $max --seed $seed > /dev/null   # warm-up
for t in "${threads[@]}"; do
  for r in $(seq 1 $runs); do
    line=$($bin --bench --spp $spp --threads $t --seed $seed --out benchRef.txt | grep '^BENCH')
    time_s=$(sed -n 's/.*time_s=\([^ ]*\).*/\1/p' <<< "$line")
    ms=$(sed -n 's/.*Msamples_per_s=\([^ ]*\).*/\1/p' <<< "$line")
    echo "$t,$r,$time_s,$ms" >> $out
  done
done
cat $out
echo
echo "median per thread count:"
awk -F, '!/^#/ && $1!="threads" {a[$1]=a[$1]" "$3} END{for(k in a){n=split(a[k],v," "); asort(v); printf "threads=%s median_time_s=%s\n",k,v[int((n+1)/2)]}}' $out | sort -t= -k2 -n
