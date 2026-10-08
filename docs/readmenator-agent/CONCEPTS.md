# Concepts

Nouns map atomically to file sets (EXTRACTED); verbs aggregate structural edges (INFERRED).

- `beacon` | files=2 | mentions=15 | `beacon.h`, `lazyc.c`
- `output` | files=2 | mentions=5 | `beacon.h`, `lazyc.c`
- `api` | files=2 | mentions=4 | `beacon.h`, `lazyc.c`
- `lazyc` | files=2 | mentions=4 | `lazyc.c`, `lazyc.s`
- `parse` | files=2 | mentions=4 | `beacon.h`, `lazyc.c`
- `error` | files=2 | mentions=3 | `beacon.h`, `lazyc.c`
- `printf` | files=2 | mentions=2 | `beacon.h`, `lazyc.c`

## Dialectic

- Thesis: `api` centralizes 2 files; Antithesis: `beacon` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `api` centralizes 2 files; Antithesis: `error` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `api` centralizes 2 files; Antithesis: `output` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `api` centralizes 2 files; Antithesis: `parse` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `api` centralizes 2 files; Antithesis: `printf` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `beacon` centralizes 2 files; Antithesis: `error` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `beacon` centralizes 2 files; Antithesis: `output` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `beacon` centralizes 2 files; Antithesis: `parse` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `beacon` centralizes 2 files; Antithesis: `printf` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
- Thesis: `error` centralizes 2 files; Antithesis: `output` pulls 2 files with 2 shared (Jaccard 1.00); Synthesis: should they merge, split by layer, or keep `bridges` explicit?
