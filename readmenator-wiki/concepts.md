# Concepts

Second-brain semantic layer: nouns map atomically to file sets (EXTRACTED); verbs aggregate structural edges (INFERRED).

| Concept | Files | Mentions | Top Files |
|---------|-------|----------|-----------|
| `beacon` | 2 | 15 | `beacon.h`, `lazyc.c` |
| `output` | 2 | 5 | `beacon.h`, `lazyc.c` |
| `api` | 2 | 4 | `beacon.h`, `lazyc.c` |
| `lazyc` | 2 | 4 | `lazyc.c`, `lazyc.s` |
| `parse` | 2 | 4 | `beacon.h`, `lazyc.c` |
| `error` | 2 | 3 | `beacon.h`, `lazyc.c` |
| `printf` | 2 | 2 | `beacon.h`, `lazyc.c` |

## Dialectic Prompts

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
