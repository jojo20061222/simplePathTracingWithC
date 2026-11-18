# Benchmark 設定與決定（Step 1：CPU baseline）

分支：`CPU-baseline`

這份文件記錄我們怎麼量測 speedup、用什麼設定，以及為什麼這樣決定。之後的 C++ 重構和 GPU 版本都要照這份設定量測，數字才能互相比較。

---

## 1. 決定摘要

| 用途 | 設定 | 說明 |
|---|---|---|
| **日常量測** | **64 spp** | 量時間和 speedup。CPU、GPU 一律用同一個 spp |
| **畫質標準（參考圖）** | **8192 spp** | 兩張 4096 spp（種子 999、1000）平均。用來判斷一張圖算得對不對 |
| **最終驗證與展示** | **8192 spp** | 專案最後量一次 speedup，並輸出報告用的圖 |

固定不變的工作量設定：

| 項目 | 值 |
|---|---|
| 場景 | `example.c` |
| 解析度 | 720 × 480（345,600 像素） |
| 每條路徑最多 bounce 數 | `maxDepth = 32`（原本的設定） |
| 俄羅斯輪盤 | 照原樣（第 5 次 bounce 起，非鏡面 bounce） |
| 量測用種子 | 42 |

---

## 2. 原版程式為什麼不能直接量

原版 `renderToWindow` 會**無限累積取樣**，直到關掉視窗才停止：

- **沒有終止條件：** 每個像素的取樣數沒有上限，而且各像素不一樣（慢的區塊取樣較少）。
- **不可重現：** 亂數種子包含 `time(NULL)`，每次執行結果都不同。

所以加了一個 `--bench` 模式：

- 每個像素剛好算 `spp` 個取樣，算完就結束。
- 種子固定。
- 只計時渲染本身（不含場景建立、讀 CSV、存檔）。

---

## 3. 終止條件

一次 benchmark 由兩層條件決定何時結束：

1. **每條路徑（原本就有，沒改）：** 什麼都沒打到、打到光源、能量趨近 0、被俄羅斯輪盤終止，或達到 `maxDepth = 32`。
2. **整張圖（新增）：** 每個像素剛好 `spp` 個取樣。所有列都算完，所有執行緒結束，計時停止。

**種子**決定用哪些亂數，也就決定了路徑長短和實際工作量。固定種子，每次執行的工作量就完全相同。

### 結果不受執行緒數影響

- 原版：每個**執行緒**一個亂數產生器，執行緒動態領 block。哪個像素拿到哪些亂數，取決於執行時的排程。
- 新版：每**一列像素**一個亂數產生器，種子由列號決定（`seed + 7919 × row + 19650218`）。每列只由一個執行緒處理，依序計算。

已驗證：1 執行緒和 7 執行緒的輸出逐位元相同；同樣設定重跑兩次也相同。

---

## 4. 為什麼日常用 64 spp

完整 spp 掃描的結果（`sppSweep.sh`，每組 3 次，1 執行緒固定在大核）：

| spp | 1 執行緒（中位數） | 20 執行緒（中位數） | speedup |
|---|---|---|---|
| 1 | 2.21 s | 0.156 s | 14.18 |
| 4 | 8.65 s | 0.603 s | 14.36 |
| 16 | 33.9 s | 2.36 s | 14.35 |
| 32 | 67.3 s | 4.70 s | 14.34 |
| **64** | **133.7 s** | **9.38 s** | **14.25** |
| 128 | 265.7 s | 18.9 s | 14.07 |

- **speedup 在 4 spp 以上就穩定**，變化在約 2% 以內。用更高的 spp 量，speedup 也一樣。
- **計時很穩定：** 所有組合的變異係數（CV）都小於 1.3%，大部分小於 0.6%。
- **時間和 spp 成正比**，工作量可控。
- 選 64 而不是更低：讓 20 執行緒有約 9 秒，GPU 快很多倍時還量得到。
- 不選 128：時間加倍，量測品質沒有提升（20 執行緒的 CV 反而從 0.6% 升到 1.3%）。

**GPU 版本出來後要重新評估：** 如果 GPU 跑 64 spp 不到約 1 秒，CPU 和 GPU 一起調到 128 或 256 spp。

---

## 5. 參考圖與正確性檢查

### 為什麼需要參考圖

| 階段 | 怎麼確認算得對 |
|---|---|
| Phase 2（C++ 重構） | 同樣種子，輸出應該**逐位元相同**，用 `cmp` 比較即可 |
| Phase 3、4（GPU） | GPU 用不同的亂數產生器（可能還改用 `float`），圖**一定不會完全相同**。改成和參考圖比較誤差 |

GPU 的判斷方法：GPU 跑 64 spp，和參考圖比較 relMAE。

- **約 0.63**（CPU 在 64 spp 的誤差）：GPU 正確，差異只是雜訊。
- **明顯更大：** 有 bug，或精度損失太多。

這也能防止「假的 speedup」：如果某個最佳化讓速度變快、畫質卻變差，誤差會變大。

### 誤差指標

誤差在**線性 XYZ** 上計算（`--out-xyz` 輸出），不用 8-bit 顯示圖。顯示轉換會把負值截成 0，再做 gamma 和 tone mapping，看不出真正的雜訊。

- **relMAE**（平均絕對誤差 ÷ 參考圖平均亮度）：**主要指標**。
- **relRMSE**：會被少數極亮的「螢火蟲」像素主導（多半是寶石的焦散），64 spp 時 90% 的誤差來自最亮的 0.1% 像素，跳動很大，僅供參考。

| spp | relMAE | relRMSE |
|---|---|---|
| 1 | 1.80 | 7.82 |
| 16 | 1.06 | 2.37 |
| **64** | **0.63** | 2.97 |
| 128 | 0.47 | 1.17 |

### 參考圖本身夠不夠準

用兩張不同種子的 4096 spp 參考圖互相比較，得到參考圖本身的雜訊：

| 指標 | 8192 spp 參考圖的雜訊 | 佔 128 spp 誤差的比例 |
|---|---|---|
| relMAE | 0.070 | 15% |
| relRMSE | 0.271 | 23% |

都在 30% 以下，用來檢查 128 spp 以下的圖足夠。

**注意：** 這張參考圖**不能**用來檢查 4096 或 8192 spp 的圖（雜訊和參考圖差不多大）。正確性一律用 64 spp 檢查。

### 參考圖檔案

位置 `sppSweep/`，**沒有加進 git**（兩個 `.xyz` 各 8 MB）：

| 檔案 | 內容 |
|---|---|
| `ref.xyz`、`ref.txt`、`ref.info` | 4096 spp，種子 999 |
| `ref2.xyz`、`ref2.txt`、`ref2.info` | 4096 spp，種子 1000 |

8192 spp 參考圖沒有存成檔案，`analyzeSpp.py` 每次執行時計算 `(ref + ref2) / 2`。

**可以完全重現：** 用同樣的程式重跑（約 20 分鐘），會得到逐位元相同的檔案。

---

## 6. 最終驗證（8192 spp）

speedup 在高 spp 不會變，兩者量到的 speedup 應該相同，差別在時間和展示圖品質：

| | 4096 spp | 8192 spp |
|---|---|---|
| 1 執行緒，一次 | 約 2.4 小時 | 約 4.7 小時 |
| 20 執行緒，一次 | 約 10 分鐘 | 約 20 分鐘 |
| 雜訊 | 基準 | 少約 30% |

做法：

- 1 執行緒只跑**一次**（CV 只有 0.1%，一次就夠）。
- 20 執行緒和 GPU 各跑 5 次。
- 報告寫明：「64 spp 量到的 speedup 和最終 spp 一致」。
- 已經有一張免費的 8192 spp CPU 圖：兩張參考圖的平均（還沒轉成 PNG）。

---

## 7. 量測機器注意事項

- **大小核 CPU：** 10 個 Cortex-X925（3.9 GHz）加 10 個 Cortex-A725（2.8 GHz）。
  - 1 執行緒的 baseline 必須用 `taskset` **固定在大核**，否則可能被排到小核，時間會變慢。`sppSweep.sh` 已經這樣做（CPU 5）。
  - 理想 speedup **不是 20 倍**，因為一半核心比較慢。目前量到的約 14.3 倍要對照這個上限解讀。
- **共用機器：** 有其他使用者。跑 benchmark 前先用 `top` 確認沒有其他重的程式在跑，跑的時候自己也不要跑其他渲染。
- **工具鏈：** 這台沒有 clang 和 SDL，用 gcc 編譯，並自動加 `-DNO_SDL`（沒有視窗，只能用 `--bench`）。

---

## 8. 怎麼跑

### 編譯

```bash
CC=gcc ./makeMakefile.sh example.c > Makefile
make all
```

### 單次 benchmark

```bash
./bin/pathTracing --bench --spp 64 --threads 20 --seed 42 --out out.txt --out-xyz out.xyz
```

輸出一行：

```
BENCH width=720 height=480 spp=64 maxDepth=32 threads=20 seed=42 time_s=... Msamples_per_s=...
```

| 選項 | 說明 | 預設 |
|---|---|---|
| `--bench` | 開啟 benchmark 模式（沒加的話和原版一樣開視窗） | |
| `--spp N` | 每像素取樣數 | 64 |
| `--threads N` | 執行緒數 | 1 |
| `--seed N` | 種子 | 42 |
| `--out FILE` | 8-bit 圖（文字格式，用 `toImg.py` 轉 PNG） | 不輸出 |
| `--out-xyz FILE` | 線性 XYZ（binary：兩個 int32 寬高，接 寬×高×3 個 float64） | 不輸出 |

1 執行緒要固定在大核：

```bash
taskset -c 5 ./bin/pathTracing --bench --spp 64 --threads 1
```

### 轉成 PNG

```bash
.venv/bin/python toImg.py out.txt out.png
```

### 執行緒數掃描

```bash
./bench.sh 64 5      # spp=64，每種執行緒數跑 5 次，結果存在 benchResults.csv
```

⚠️ `bench.sh` 目前**還沒有**把 1 執行緒固定在大核（待修）。

### spp 掃描（重新產生參考圖和第 4、5 節的數據）

```bash
./sppSweep.sh                                # 約 50 分鐘，輸出到 sppSweep/
.venv/bin/python analyzeSpp.py sppSweep      # 表格、summary.csv、sppSweep.png
```

可以用環境變數調整：`REF_SPP`、`MAX_SPP`、`RUNS`、`THREADS`、`SEED`、`REF_SEED`、`REF_SEED2`、`PIN_SINGLE`、`OUTDIR`。說明在 `sppSweep.sh` 開頭。

`sppSweep.sh` 跑很久，SSH 可能會斷線，建議脫離終端機執行：

```bash
setsid nohup ./sppSweep.sh > sppSweep/run.log 2>&1 < /dev/null &
tail sppSweep/run.log    # 最後一行出現 "done." 就是跑完了
```

### Python 環境

```bash
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt    # pillow、numpy、matplotlib
```

---

## 9. 程式改動一覽

渲染核心（`src/render.c` 的 `renderPixel`、`tracePath`）**沒有改**。

| 檔案 | 改動 |
|---|---|
| `example.c` | `main` 讀命令列參數；有 `--bench` 就呼叫 `renderBenchmark` |
| `src/pathTracing.c`、`include/pathTracing.h` | 新增 `renderBenchmark`：計時、印 `BENCH` 行、輸出圖；SDL 部分可用 `NO_SDL` 排除 |
| `src/multithreads.c`、`.h` | 新增 `runFixedWorkload`：以列為單位、固定 spp、每列獨立種子。原本的函式沒改 |
| `src/render.c`、`.h` | 新增 `getMeanXYZ`，讀取像素的線性 XYZ 平均值 |
| `makeMakefile.sh` | `CC` 可指定；沒有 SDL 時自動加 `-DNO_SDL` |
| `bench.sh`、`sppSweep.sh`、`analyzeSpp.py` | 量測和分析腳本 |
| `requirements.txt` | 加 numpy、matplotlib |

---

## 10. 待決定 / 待辦

- [x] 最終驗證與展示用 **8192 spp**
- [ ] GPU 版本出來後，確認 64 spp 是否需要調高
- [ ] `bench.sh` 加上 1 執行緒固定大核
- [ ] 把兩張參考圖平均成 8192 spp，輸出 `.xyz` 和 `.png`（報告用）
- [ ] 決定 `sppSweep/`、`benchResults.csv`、`benchRef.txt` 是否加進 `.gitignore`
- [ ] 下一個 Phase（C++ 重構）開新分支
