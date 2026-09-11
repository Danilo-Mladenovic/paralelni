# Paralelni sistemi — priprema za ispit

Zbirka rešenih zadataka, cheatsheet-ova i teorijskih materijala za predmet **Paralelni sistemi (PS)** — MPI, OpenMP i CUDA.

## Struktura

| Folder / fajl | Sadržaj |
|---|---|
| [MPI/](MPI/) | MPI zadaci sa rokova (`2020_septembar.cpp` … `2024_januar.cpp`) |
| [MPI/MPI_CHEATSHEET.md](MPI/MPI_CHEATSHEET.md) | MPI cheatsheet |
| [OPEN_MP/](OPEN_MP/) | OpenMP zadaci sa rokova (`2021_oktobar.cpp` … `2026_jan.cpp`) |
| [OPEN_MP/openmp_cheatsheet.md](OPEN_MP/openmp_cheatsheet.md) | OpenMP cheatsheet |
| [CUDA/](CUDA/) | CUDA zadaci sa rokova (`2023_jun.cpp` … `2025_oktobar.cpp`) |
| [claude/](claude/) | Teorija po temama + tipovi pismenih zadataka (markdown) |
| [blanketi/](blanketi/) | Skenirani blanketi sa rokova (`p` = pismeni, `u` = usmeni) |
| [functions.cpp](functions.cpp) | Deklaracije MPI funkcija za brzo podsećanje |
| [mpi_files_tips.txt](mpi_files_tips.txt) | Napomene za MPI I/O (kada koristiti koju funkciju) |
| [PS_teorijska_pitanja_sa_rokova.md](PS_teorijska_pitanja_sa_rokova.md) | Teorijska pitanja sa prethodnih rokova |
| `Deklaracije fja 2021.pdf` | Zvanični spisak deklaracija funkcija (dozvoljen na ispitu) |
| `PS CRNO I ZLATNO skripta.pdf` | Skripta za predmet |

## Konvencija imenovanja

Zadaci su imenovani po roku: `<godina>_<mesec>.cpp` (npr. `2025_jun.cpp`).
Sufiks `_v2`, `_v3` označava alternativnu varijantu rešenja istog zadatka,
a `_z2`, `_z3` / `_2` konkretan broj zadatka sa tog roka.
U [blanketi/](blanketi/) sufiks `- p` je pismeni deo, `- u` usmeni, `- pu` oba.

## Kompajliranje

```bash
# MPI
mpic++ MPI/2023_april.cpp -o zadatak && mpirun -np 4 ./zadatak

# OpenMP
g++ -fopenmp OPEN_MP/2025_jun.cpp -o zadatak && ./zadatak

# CUDA (fajlovi su .cpp, ali sadrže CUDA kod)
nvcc -x cu CUDA/2025_oktobar.cpp -o zadatak && ./zadatak
```

> Napomena: zadaci sa rokova se pišu na papiru, pa dosta fajlova nije sintaksno
> kompletno ni kompajlabilno — služe kao šablon i podsetnik na obrazac rešavanja.

## Teorija

U [claude/](claude/) su obrađene teme:
[Amdahl-ov zakon](claude/amdahl_teorija.md),
[Bernstajnovi uslovi / SP / VLIW / banke memorije](claude/bernstajn_sp_vliw_banci.md),
[keš koherencija](claude/kes_koherencija_teorija.md),
[tipovi zavisnosti](claude/zavisnosti_teorija.md),
[mreže za povezivanje](claude/mreze_teorija.md),
te teorija ([MPI](claude/mpi_pismeni_tipovi.md) · [OpenMP](claude/openmp_teorija.md) · [CUDA](claude/cuda_teorija.md))
i tipovi pismenih zadataka ([MPI](claude/mpi_pismeni_tipovi.md) · [OpenMP](claude/openmp_pismeni_tipovi.md) · [CUDA](claude/cuda_pismeni_tipovi.md)).
