# Paralelni sistemi — priprema za ispit

Zbirka rešenih zadataka, cheatsheet-ova i teorijskih materijala za predmet **Paralelni sistemi (PS)** — MPI, OpenMP i CUDA.

## Struktura

| Folder / fajl                                                          | Sadržaj                                                    |
| ---------------------------------------------------------------------- | ---------------------------------------------------------- |
| [MPI/](MPI/)                                                           | Zadaci sa rokova rađeni samostalno (`2023_april.cpp`, …)   |
| [MPI/resenja/](MPI/resenja/)                                           | Zvanična / tuđa rešenja MPI zadataka                       |
| [MPI/radjeni_zadaci/](MPI/radjeni_zadaci/)                             | Dodatni prorađeni MPI zadaci                               |
| [MPI/MPI_CHEATSHEET.md](MPI/MPI_CHEATSHEET.md)                         | MPI cheatsheet                                             |
| [OPEN_MP/](OPEN_MP/)                                                   | OpenMP zadaci sa rokova                                    |
| [OPEN_MP/resenja/](OPEN_MP/resenja/)                                   | Zvanična rešenja OpenMP zadataka                           |
| [OPEN_MP/openmp_cheatsheet.md](OPEN_MP/openmp_cheatsheet.md)           | OpenMP cheatsheet                                          |
| [CUDA/](CUDA/)                                                         | CUDA zadaci sa rokova                                      |
| [CUDA/resenja/](CUDA/resenja/)                                         | Zvanična rešenja CUDA zadataka (`.cu` / `.cpp`)            |
| [claude/](claude/)                                                     | Teorija po temama + tipovi pismenih zadataka (markdown)    |
| [blanketi/](blanketi/)                                                 | Skenirani blanketi sa rokova (`p` = pismeni, `u` = usmeni) |
| [usmeni/](usmeni/)                                                     | Skripte i sabrana pitanja za usmeni                        |
| [functions.cpp](functions.cpp)                                         | Deklaracije MPI/OpenMP/CUDA funkcija za brzo podsećanje    |
| [mpi_files_tips.txt](mpi_files_tips.txt)                               | Napomene za MPI I/O (kada koristiti koju funkciju)         |
| [PS_teorijska_pitanja_sa_rokova.md](PS_teorijska_pitanja_sa_rokova.md) | Teorijska pitanja sa prethodnih rokova                     |

## Konvencija imenovanja

Zadaci su imenovani po roku: `<godina>_<mesec>.cpp` (npr. `2025_jun.cpp`).
Sufiks `_v2`, `_v3` označava alternativnu varijantu rešenja istog zadatka.
U `blanketi/` sufiks `- p` je pismeni deo, `- u` usmeni, `- pu` oba.

## Kompajliranje

```bash
# MPI
mpic++ MPI/2023_april.cpp -o zadatak && mpirun -np 4 ./zadatak

# OpenMP
g++ -fopenmp OPEN_MP/2025_jun.cpp -o zadatak && ./zadatak

# CUDA
nvcc CUDA/resenja/2025_septembar.cu -o zadatak && ./zadatak
```

> Napomena: zadaci iz rokova se pišu na papiru, pa dosta fajlova nije sintaksno
> kompletno ni kompajlabilno — služe kao šablon i podsetnik na obrazac rešavanja.

## Teorija

U [claude/](claude/) su obrađene teme:
Amdahl-ov zakon, Bernstajnovi uslovi / SP / VLIW / banke memorije,
keš koherencija, tipovi zavisnosti, mreže za povezivanje,
i teorija + tipovi zadataka za MPI, OpenMP i CUDA.
