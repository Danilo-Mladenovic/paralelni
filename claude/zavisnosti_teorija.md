# Zavisnosti u petljama — tipovi zadataka i kako se razmišlja

# 0. Osnova

**Zavisnost postoji** kad dve naredbe pristupaju **istoj memorijskoj lokaciji** i bar jedna
piše. Vrste:

| Vrsta | Šta se dešava | Sme li se redosled menjati |
|---|---|---|
| **RAW** (prava) | piše pa čita | NE |
| **WAR** (anti) | čita pa piše | NE |
| **WAW** (izlazna) | piše pa piše | NE |

**Loop-carry** = zavisnost je **između različitih iteracija** → blokira paralelizaciju.
**Loop-independent** = unutar **iste** iteracije → **ne smeta**, svaka iteracija je samostalna.

## Vektor zavisnosti

```
d = (iteracija PONORA) − (iteracija IZVORA)
```
komponenta po komponenta, redom **od spoljne ka unutrašnjoj** petlji.

**Kako ga praktično nađeš — tri koraka:**
1. Nađi u kojoj se iteraciji element **upisuje**
2. Nađi u kojoj se **čita** (ili ponovo upisuje)
3. Oduzmi: kasnija minus ranija

**Pravac:** `d_k > 0 → <`, `d_k = 0 → =`, `d_k < 0 → >`

**Vektor mora biti pozitivan** (prva nenulta komponenta > 0). Ako ispadne negativan —
zamenio si izvor i ponor. Okreni znak; to je onda anti-zavisnost (WAR).

## Jedno pravilo iz kog sve sledi

> **Zavisnost nosi petlja na poziciji PRVE NENULTE komponente vektora (sleva).**

Iz njega direktno:
> **Petlja L se sme paralelizovati/vektorizovati ⟺ NIJEDAN vektor nema prvu nenultu
> komponentu baš na poziciji L.**

---

# 1. TIP: klasifikuj subscript parove (ZIV/SIV/MIV) i separabilnost

**Kako razmišljaš:** gledaš **poziciju po poziciju** u indeksu niza. Za svaku poziciju uzmeš
par (izraz u upisu, izraz u čitanju) i **prebrojiš koliko različitih iterativnih promenljivih
se u tom paru javlja**:

| Broj promenljivih | Tip |
|---|---|
| 0 (samo konstante) | **ZIV** |
| 1 | **SIV** |
| ≥2 | **MIV** |

*Primer:* `A(2i+1, 5, j) = A(2i−2, N, k)`
→ poz.1 `(2i+1, 2i−2)` = **SIV**; poz.2 `(5, N)` = **ZIV**; poz.3 `(j, k)` = **MIV**

**Separabilnost — drugo pitanje, druga provera.** Gledaš **jednu referencu** i pitaš se:
**javlja li se ista promenljiva na više pozicija?**
- ne javlja → pozicija je **separabilna** → sme se testirati **nezavisno**
- javlja se → te pozicije su **povezane (coupled)** → moraju se analizirati **zajedno**

*Primer:* `A(2i+1, 5, i)` — `i` je na poziciji 1 **i** 3 → te dve pozicije su povezane.

**Zašto je separabilnost bitna:** ako su sve pozicije separabilne, dovoljno je da **jedna**
dokaže „nema zavisnosti" i cela referenca je čista. Kod povezanih to ne važi — rešenje po `i`
iz jedne pozicije mora biti konzistentno sa drugom.

---

# 2. TIP: ZIV test

Par `(c₁, c₂)` gde su oba **konstante**:
```
c₁ ≠ c₂  ⟹  zavisnosti SIGURNO NEMA     (dve različite pozicije, nikad se ne poklope)
c₁ = c₂  ⟹  zavisnost postoji
```
Ovo je **egzaktan** test — daje definitivan odgovor.

**Zamka koja se stalno pita:** ako je jedna vrednost **simbolička** (`N`, granica petlje) —
**ne možeš zaključiti ništa**. `(5, N)` ne dokazuje odsustvo zavisnosti, jer N može biti baš
5. Moraš se osloniti na drugu poziciju.

---

# 3. TIP: GCD test

Za par `a·I + b` (upis) i `c·I + d` (čitanje):
```
NZD(a, c)  deli  (d − b)  ?
```
- **NE deli** → zavisnosti **SIGURNO NEMA** ← ovo je jedini jak zaključak
- **deli** → zavisnost **MOŽE, ali ne mora** postojati

**Test je dovoljan, ne i potreban.** „Deli" ne dokazuje ništa — samo znači da nisi uspeo da
isključiš zavisnost.

**Praktično, za više promenljivih:**
```
NZD(SVI koeficijenti oba izraza)  deli  (razlika slobodnih članova)?
```

**Kako brzo prepoznaš ishod:** ako je **bilo koji** koeficijent neparan (ili se dva razlikuju
za 1), NZD = 1, a **1 deli sve** → uvek ispadne „može postojati". Test isključi zavisnost tek
kad su **svi** koeficijenti parni (NZD ≥ 2) a razlika konstanti neparna.

*Primer gde test radi:* `a(4i + 2j + 1)` vs `a(6i + 2k + 4)` → NZD(4,2,6,2) = **2**,
razlika = 4−1 = **3**, 2 ne deli 3 → **nema zavisnosti**.

**Normalizacija:** formalno se petlja svodi na početak od 0 pre testa, ali **rezultat se ne
menja** — pomeranje granica ne dira ni koeficijente ni razliku konstanti.

---

# 4. TIP: nađi vektore zavisnosti i reci šta se sme paralelizovati

Ovo je **glavni tip**. Tok razmišljanja:

### Korak 1 — popiši SVE parove upis↔pristup
Za svaki niz koji se i piše i čita, i za **svaki** izraz u kome se javlja. Ako se u telu
javljaju tri reference na `A`, imaš do tri vektora. **Ne zaboravi upis↔upis (WAW).**

### Korak 2 — za svaki par nađi vektor
Ključno pitanje: **u kojoj se iteraciji upisuje element koji se ovde čita?**

Postupak: izjednači indeks upisa sa indeksom čitanja i reši po iterativnim promenljivama.
*Primer:* upis `A[i+1][j−2]` u iteraciji `(j,i)`; čitanje `A[i][j]`. Element `(i,j)` upisuje
se kad je `i_w+1 = i` i `j_w−2 = j`, dakle u iteraciji `(j+2, i−1)`. To je **kasnije** →
WAR → vektor od čitanja ka upisu: `d = (2, −1)`.

### Korak 3 — pretvori u pravce i odredi ko nosi
Prva nenulta komponenta sleva → ta petlja nosi zavisnost.

### Korak 4 — odgovori na pitanje
- „može li se petlja L paralelizovati?" → **da li ijedan vektor ima prvu nenultu komponentu
  na poziciji L?**
- „može li se unutrašnja vektorizovati?" → isto pitanje za poslednju poziciju

## Dve zamke koje se najviše naplaćuju

**Zamka 1 — upis u istu lokaciju u svakoj iteraciji.**
```c
for (j...) for (i...) { B(j+1) = ...; }     // i se ne javlja u indeksu B!
```
`B(j+1)` se upisuje u **svakoj** i-iteraciji → **WAW koju nosi i** → vektor `(0, +)`.
Lako se previdi jer se `i` uopšte ne pojavljuje u indeksu. **Uvek proveri da li se neka
promenljiva NE javlja u indeksu — to je znak za WAW po toj petlji.**

**Zamka 2 — promenljiva se ne javlja ni u jednom indeksu.**
Ako iterativna promenljiva ne učestvuje u indeksima niza, na njenoj poziciji stoji `*`
(svi pravci mogući) → mora se razmatrati najgori slučaj.

---

# 5. TIP: nekonstantna komponenta vektora

Prepoznaje se po tome što ti komponenta ispadne **izraz sa `i`**, a ne broj (npr. `2i−100`,
`4−i`). Uzrok je koeficijent ≠ 1 uz iterativnu promenljivu (`A(100−i)`, `A(2i−4)`).

**Kako razmišljaš:** to nije jedan vektor nego **skup vektora**. Razmotri **sve tri
mogućnosti** i navedi ih sve:
```
izraz < 0  →  pravac >
izraz = 0  →  pravac =
izraz > 0  →  pravac <
```
Nađi prelomnu vrednost `i` (gde izraz postaje nula) i podeli opseg.

*Primer:* `d = (1, 2i−100)` za i∈[1,99] → i<50: `(<, >)`; i=50: `(<, =)`; i>50: `(<, <)`.

**Zašto je bitno:** slučaj gde komponenta **postane nula** menja **ko nosi zavisnost**. Ako je
prva komponenta `4−i` i za `i=4` ispadne 0, onda za tu vrednost zavisnost prelazi na
**sledeću** petlju. Zato se ne sme reći „ovu petlju nosi i" bez provere svih slučajeva.

---

# 6. TIP: transformacije gnezda petlji

Tri elementarne, sve imaju **|det T| = 1**:

| Transformacija | Šta radi | Matrica |
|---|---|---|
| **Permutacija** | zamenjuje dve petlje mestima | jedinična sa zamenjene dve vrste |
| **Obrtanje** | okreće smer jedne petlje | jedinična sa **−1** na toj dijagonali |
| **Krivljenje** | krivi jedan indeks u odnosu na drugi za faktor f | jedinična sa **f** van dijagonale |

## Dozvoljenost — jedino pravilo koje treba
```
d > 0   ⟹   T·d > 0        za SVAKI vektor zavisnosti
```
Ako transformacija bilo koji vektor **okrene u negativan**, ona bi obrnula redosled
izračunavanja → **nije dozvoljena**.

## Tok razmišljanja kad se traži „omogući vektorizaciju"

1. **Nađi petlju koja ne nosi nijednu zavisnost.** Prođi kroz vektore i traži poziciju na
   kojoj nijedan vektor nema svoju prvu nenultu komponentu.
2. **Ako takva petlja postoji** → permutuj je na **unutrašnju** poziciju. Proveri dozvoljenost.
3. **Ako permutacija padne** (neki vektor postane negativan) → kombinuj:
   - **permutacija + obrtanje** — obrtanje popravlja znak
   - **krivljenje pa permutacija** — krivljenje najpre ukloni nulu iz spoljne komponente
4. **Ako nijedna petlja nije slobodna** → krivljenje sa faktorom f: `T·d` menja spoljnu
   komponentu iz 0 u nešto pozitivno, čime se zavisnost „prebacuje" na spoljnu petlju.

**Praktičan trik:** kad tražiš da li permutacija prolazi, ne moraš množiti matrice — samo
**zameni komponente vektora mestima** i pogledaj je li prva nenulta i dalje pozitivna.

## Kako se piše transformisana petlja

Iz smene promenljivih izrazi **stare preko novih**, pa zameni svuda:
- **obrtanje po j:** `v = −j` → `j = −v`, granice j∈[1,n] postaju v∈[−n,−1]
- **permutacija:** samo zameni redosled `for` linija
- **krivljenje j po i sa f:** `v = f·i + j` → `j = v − f·i`, granice postaju **trougaone**
  (`v` ide od `f·u+donja` do `f·u+gornja`)

---

# 7. TIP: eliminiši zavisnost preuređenjem koda

Ovde nema matrica — gleda se **struktura koda**.

## 7a — je li zavisnost uopšte loop-carry?
```c
S1: a[i] = b[i] + C;
S2: d[i] = a[i] + E;      // čita a[i] iz ISTE iteracije
```
Ovo je **loop-independent** RAW (pravac `=`) → **ne smeta**, petlja je već paralelizabilna.

**Zamka:** zadatak ponudi „preuređenje" koje pomera naredbu za jednu iteraciju — time se
**uvodi loop-carry zavisnost koje pre nije bilo**. Odgovor: preuređenje je **pogrešno**,
original je već bio paralelizabilan.

## 7b — WAW između dve naredbe u istoj petlji
```c
S1: a[i]   = b[i+1] * c;
S2: a[i+1] = b[i]   * d;     // S1 sledeće iteracije prepisuje isto mesto
```
`a[i+1]` upisuje S2 u iteraciji `i`, pa ga S1 **prepisuje** u iteraciji `i+1` → **WAW**,
nije paralelizabilno kako jeste.

**Kako se rešava:** pitaj se **koji upis preživi**. Ovde svi upisi iz S2 budu prepisani —
osim **poslednjeg** (koji S1 ne stigne da prepiše). Ekvivalentan kod: paralelna petlja samo
sa S1, pa jedan upis van petlje.

## 7c — zavisnost sa obrnutim smerom u dve polovine opsega
```c
for (i = 1; i < N; i++)  A(i) = A(N−i+1) + C;
```
Za malo `i` element se upisuje **kasnije** (WAR), za veliko `i` je upisan **ranije** (RAW).
Prelom je na sredini.

**Rešenje — podela petlje na dve polovine.** U svakoj polovini se opsezi upisanih i čitanih
elemenata **ne preklapaju** → svaka je nezavisna i vektorizabilna, a redosled dve petlje
čuva semantiku.

---

# 8. Kontrolna lista

- Jesi li popisao **sve** parove, uključujući **upis↔upis (WAW)**?
- Javlja li se neka iterativna promenljiva u telu, a **ne** u indeksu niza? → WAW po toj petlji
- Je li ijedna komponenta vektora **izraz sa i**? → razmotri sva tri slučaja
- Je li vektor **pozitivan**? Ako nije, zamenio si izvor i ponor
- Kod GCD-a — jesi li zaključio samo iz „NE deli"? („deli" ne dokazuje ništa)
- Kod ZIV-a — je li neka vrednost **simbolička**? Onda nema zaključka
- Kod transformacije — jesi li proverio dozvoljenost za **svaki** vektor, ne samo za jedan?
- Je li zavisnost uopšte **loop-carry**, ili je unutar iste iteracije (pa ne smeta)?
