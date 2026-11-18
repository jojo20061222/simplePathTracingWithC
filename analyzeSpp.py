#!/usr/bin/env python3
"""Analyze the output of sppSweep.sh.

Usage: .venv/bin/python analyzeSpp.py [outdir=sppSweep]
Needs numpy; matplotlib is optional (for the plot).

Per spp it reports:
  - relRMSE: RMSE of the linear XYZ image against the reference, divided by the reference's mean
    value. This is the error metric to use: it is measured before the display conversion, which
    clamps negative RGB to 0, applies gamma and tone mapping, and so hides the true noise level.
    Expect relRMSE ~ 1/sqrt(spp), i.e. a log-log slope of about -0.5.
  - relMAE: mean absolute error / reference mean. Less sensitive to outliers than relRMSE.
  - top0.1%: share of the squared error coming from the worst 0.1% of pixels. A high value means
    a few "firefly" pixels (bright caustics through the gems) dominate relRMSE, which then falls
    unevenly; look at relMAE as well in that case.
  - rmse8: RMSE of the 8-bit display image (0-255), for reference only.
  - median time and timing variation (CV = stdev / mean) per thread count
  - speedup = median time with the fewest threads / median time with the most threads
If ref2.xyz exists (a second reference with the same spp and another seed), the reference used is
the average (ref + ref2) / 2, which is statistically identical to one image with twice the spp.
Its own noise is then error(ref, ref2) / 2; it should be well below the error of the test images.
Writes <outdir>/summary.csv, and <outdir>/sppSweep.png when matplotlib is installed.
"""
import csv
import math
import os
import statistics
import sys

import numpy as np


def load_xyz(path):
    with open(path, "rb") as f:
        w, h = np.fromfile(f, dtype=np.int32, count=2)
        data = np.fromfile(f, dtype=np.float64, count=int(w) * int(h) * 3)
    return int(w), int(h), data.reshape(-1, 3)


def load_rgb8(path):
    with open(path) as f:
        w, h = map(int, f.readline().split())
        p = np.loadtxt(f, dtype=np.uint64)
    return w, h, np.stack([p & 255, (p >> 8) & 255, (p >> 16) & 255], axis=1).astype(float)


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else "sppSweep"
    path = lambda name: os.path.join(outdir, name)
    if not os.path.exists(path("ref.xyz")):
        sys.exit(f"{path('ref.xyz')} not found: run sppSweep.sh first")
    w, h, ref = load_xyz(path("ref.xyz"))
    ref_count = 1
    ref_noise = None
    if os.path.exists(path("ref2.xyz")):
        w2, h2, ref2 = load_xyz(path("ref2.xyz"))
        if (w2, h2) != (w, h):
            sys.exit(f"ref2.xyz: size {w2}x{h2} differs from ref.xyz {w}x{h}")
        both = np.isfinite(ref).all(axis=1) & np.isfinite(ref2).all(axis=1)
        diff = (ref - ref2)[both]
        ref = (ref + ref2) / 2  # = one reference with twice the samples
        ref_count = 2
    ref_finite = np.isfinite(ref).all(axis=1)
    ref_scale = ref[ref_finite].mean()
    if ref_count == 2:
        # var(ref - ref2) = 2 s^2 and var of the average = s^2 / 2, so its noise is |diff| / 2
        ref_noise = {"relRMSE": float(np.sqrt(np.mean(diff ** 2)) / 2 / ref_scale),
                     "relMAE": float(np.mean(np.abs(diff)) / 2 / ref_scale)}
    ref8 = load_rgb8(path("ref.txt"))[2] if os.path.exists(path("ref.txt")) else None


    # timings: {spp: {threads: [time, ...]}}
    times = {}
    with open(path("timings.csv")) as f:
        for row in csv.DictReader(line for line in f if not line.startswith("#")):
            times.setdefault(int(row["spp"]), {}).setdefault(int(row["threads"]), []).append(
                float(row["time_s"]))
    spps = sorted(times)
    thread_counts = sorted({t for d in times.values() for t in d})
    t_lo, t_hi = thread_counts[0], thread_counts[-1]

    results = []
    for spp in spps:
        row = {"spp": spp, "relRMSE": None, "relMAE": None, "top0.1%": None, "rmse8": None}
        if os.path.exists(path(f"img_spp{spp}.xyz")):
            iw, ih, img = load_xyz(path(f"img_spp{spp}.xyz"))
            if (iw, ih) != (w, h):
                sys.exit(f"img_spp{spp}.xyz: size {iw}x{ih} differs from reference {w}x{h}")
            ok = ref_finite & np.isfinite(img).all(axis=1)
            diff = img[ok] - ref[ok]
            row["relRMSE"] = float(np.sqrt(np.mean(diff ** 2)) / ref_scale)
            row["relMAE"] = float(np.mean(np.abs(diff)) / ref_scale)
            sq = np.sort((diff ** 2).sum(axis=1))[::-1]
            row["top0.1%"] = float(sq[: max(1, len(sq) // 1000)].sum() / sq.sum()) if sq.sum() else 0.0
        if ref8 is not None and os.path.exists(path(f"img_spp{spp}.txt")):
            row["rmse8"] = float(np.sqrt(np.mean((load_rgb8(path(f"img_spp{spp}.txt"))[2] - ref8) ** 2)))
        for t in thread_counts:
            ts = times[spp].get(t, [])
            if ts:
                mean = statistics.mean(ts)
                row[f"median_t{t}"] = statistics.median(ts)
                row[f"cv_t{t}"] = statistics.stdev(ts) / mean if len(ts) > 1 and mean > 0 else None
        if len(thread_counts) > 1 and f"median_t{t_lo}" in row and f"median_t{t_hi}" in row:
            row["speedup"] = row[f"median_t{t_lo}"] / row[f"median_t{t_hi}"]
        results.append(row)

    # least-squares slope of log(relRMSE) vs log(spp)
    e = [r for r in results if r["relRMSE"]]
    slope = None
    if len(e) >= 2:
        slope = float(np.polyfit(np.log([r["spp"] for r in e]),
                                 np.log([r["relRMSE"] for r in e]), 1)[0])

    fmt = lambda v, s: "-" if v is None else s.format(v)
    header = f"{'spp':>5} {'relRMSE':>8} {'relMAE':>7} {'top0.1%':>7} {'rmse8':>7}"
    for t in thread_counts:
        header += f" {'t' + str(t) + ' med(s)':>13} {'CV':>6}"
    if len(thread_counts) > 1:
        header += f" {'speedup':>8}"
    print(header)
    for r in results:
        line = (f"{r['spp']:>5} {fmt(r['relRMSE'], '{:8.4f}'):>8} {fmt(r['relMAE'], '{:7.4f}'):>7}"
                f" {fmt(r['top0.1%'], '{:7.1%}'):>7} {fmt(r['rmse8'], '{:7.2f}'):>7}")
        for t in thread_counts:
            line += f" {fmt(r.get(f'median_t{t}'), '{:13.3f}'):>13}"
            line += f" {fmt(r.get(f'cv_t{t}'), '{:6.1%}'):>6}"
        if len(thread_counts) > 1:
            line += f" {fmt(r.get('speedup'), '{:8.2f}'):>8}"
        print(line)
    print()
    print(f"relRMSE log-log slope: {fmt(slope, '{:.3f}')} (expect about -0.5)")
    ref_info = open(path("ref.info")).read().strip() if os.path.exists(path("ref.info")) else "?"
    if ref_count == 2:
        ref_spp = ref_info.split()[0].removeprefix("spp=")
        eff = f"{2 * int(ref_spp)} spp" if ref_spp.isdigit() else "2x spp"
        print(f"reference: average of ref + ref2 ({ref_info} each) = {eff}")
    else:
        print(f"reference: {ref_info}")
    if ref_noise:
        # relMAE does not scale exactly like this, but it is a fair sanity check
        print(f"reference's own noise (|ref - ref2| / 2): relRMSE={ref_noise['relRMSE']:.4f} "
              f"relMAE={ref_noise['relMAE']:.4f}")
        if e:
            last = e[-1]
            print(f"  vs spp={last['spp']}: relRMSE={last['relRMSE']:.4f} relMAE={last['relMAE']:.4f}"
                  f" -> reference noise is {ref_noise['relRMSE'] / last['relRMSE']:.0%} / "
                  f"{ref_noise['relMAE'] / last['relMAE']:.0%} of it (want well under ~30%)")
    else:
        print("no ref2.xyz: reference noise not measured")

    keys = ["spp", "relRMSE", "relMAE", "top0.1%", "rmse8"]
    keys += [k for t in thread_counts for k in (f"median_t{t}", f"cv_t{t}")]
    if len(thread_counts) > 1:
        keys.append("speedup")
    with open(path("summary.csv"), "w", newline="") as f:
        wr = csv.DictWriter(f, fieldnames=keys)
        wr.writeheader()
        for r in results:
            wr.writerow({k: r.get(k) for k in keys})
    print(f"wrote {path('summary.csv')}")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed: skipping plot")
        return

    fig, axes = plt.subplots(1, 3, figsize=(15, 4.2))
    ax = axes[0]
    if e:
        xs = [r["spp"] for r in e]
        ax.loglog(xs, [r["relRMSE"] for r in e], "o-", label="relRMSE (linear XYZ)")
        ax.loglog(xs, [r["relMAE"] for r in e], "s-", label="relMAE (linear XYZ)")
        if ref_noise:
            ax.axhline(ref_noise["relRMSE"], color="C0", ls=":", label="reference noise (relRMSE)")
            ax.axhline(ref_noise["relMAE"], color="C1", ls=":", label="reference noise (relMAE)")
        c = e[0]["relRMSE"] * math.sqrt(e[0]["spp"])
        ax.loglog(xs, [c / math.sqrt(x) for x in xs], "--", label="1/sqrt(spp)")
        ax.legend(fontsize=8)
    ax.set(xlabel="spp", ylabel="relative error vs reference", title="Image error")
    ax = axes[1]
    for t in thread_counts:
        pts = [(r["spp"], r[f"median_t{t}"]) for r in results if f"median_t{t}" in r]
        ax.loglog(*zip(*pts), "o-", label=f"{t} threads")
    ax.set(xlabel="spp", ylabel="median time (s)", title="Render time")
    ax.legend()
    ax = axes[2]
    sp = [(r["spp"], r["speedup"]) for r in results if r.get("speedup")]
    if sp:
        ax.semilogx(*zip(*sp), "o-")
    ax.set(xlabel="spp", ylabel=f"speedup ({t_lo} -> {t_hi} threads)", title="Speedup")
    for ax in axes:
        ax.grid(True, which="both", alpha=0.3)
    fig.tight_layout()
    fig.savefig(path("sppSweep.png"), dpi=120)
    print(f"wrote {path('sppSweep.png')}")


if __name__ == "__main__":
    main()
