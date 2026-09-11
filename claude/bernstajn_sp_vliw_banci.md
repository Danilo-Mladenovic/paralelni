# Bernštajn, SP vs VLIW, memorijski banci — tipovi zadataka i kako se razmišlja

---

# DEO 1 — Bernštajnovi uslovi

## 0. Osnova

Za svaki proces (blok naredbi) formiraš **dva skupa**:

- **I** (input) — sve promenljive koje proces **čita** (desna strana dodele)
- **O** (output) — sve promenljive koje proces **upisuje** (leva strana dodele)

**Skupovi se prave za CEO proces** — unija svih naredbi u njemu, ne naredbu po naredbu.

## Uslov paralelnosti

Dva procesa mogu paralelno **⟺ sva tri preseka su prazna**:
```
I₁ ∩ O₂ = ∅        (P₁ ne čita ono što P₂ piše)
I₂ ∩ O₁ = ∅        (P₂ ne čita ono što P₁ piše)
O₁ ∩ O₂ = ∅        (ne pišu istu promenljivu)
```

**Ovo su bukvalno tri vrste zavisnosti:** prva dva su RAW/WAR, treći je WAW. Isti pojmovi kao
kod petlji, samo primenjeni na blokove naredbi umesto na iteracije.

**Bar jedan presek neprazan → NE MOGU.** Dovoljno je naći **jedan** da staneš.

## TIP: „koje naredbe se mogu izvršavati paralelno"

**Tok razmišljanja:**
1. Napiši I i O za svaki proces
2. Testiraj **svaki par posebno** (za 3 procesa → 3 para, za 5 → 10 parova)
3. Zaključak je **po parovima**, nikad „svi ili niko"

**Zamka 1:** promenljiva koja se unutar **istog** procesa i piše i čita (`C = D*E;  M = G+C;`)
ide u **oba** skupa — to je zavisnost unutar procesa, ne smeta paralelizaciji sa drugima.

**Zamka 2:** tipičan ishod je da P₁ ne može ni sa P₂ ni sa P₃, **ali P₂ i P₃ mogu međusobno**
— jer oba samo **čitaju** ono što je P₁ upisao. Dva čitaoca se nikad ne sukobljavaju.

## TIP: maksimalni paralelizam, vreme, ubrzanje

Kad je dat niz naredbi (S1…S5) i traži se vreme i ubrzanje:

1. **Testiraj sve parove** → dobiješ spisak zavisnih parova
2. **Nacrtaj graf prethođenja** — strelica od ranije ka kasnijoj naredbi za svaki zavisan par
3. **Rasporedi po nivoima:** naredba ide u najraniji trenutak u kom su joj svi prethodnici
   gotovi
4. **Vreme = broj nivoa** (ako svaka naredba traje 1 jedinicu)
5. **Ubrzanje = broj_naredbi / broj_nivoa**
6. **Broj potrebnih procesora** = najveći broj naredbi na jednom nivou

**Ključno:** vreme je ograničeno **kritičnim putem** — najdužim lancem zavisnosti u grafu.
Dodavanje procesora preko maksimalne širine grafa **ne pomaže ništa**.

---

# DEO 2 — Superskalarni vs VLIW

Ovde postoji samo jedno pitanje, i svodi se na **jednu rečenicu**:

> **VLIW = uvek i SAMO statičko planiranje. SP = i statičko i dinamičko.**

| | Superskalarni | VLIW |
|---|---|---|
| Broj instrukcija po ciklusu | **varijabilan** | **fiksan** |
| Ko grupiše instrukcije | hardver, u toku izvršavanja | **kompajler, unapred** |
| Planiranje | statičko **I/ILI** dinamičko | **samo statičko** |
| Složenost hardvera | veća | manja |

**Odatle odgovori na tipično zaokruživanje:**
- „statički uz pomoć kompajlera" → **SP i VLIW**
- „dinamički u fazi izvršenja" → **samo SP**
- „moguće su obe varijante" → **samo SP**

**Logika iza toga:** kod VLIW-a je broj slotova u paketu deo same arhitekture instrukcija, pa
hardver **nema šta da odlučuje** — sve je fiksirano pre izvršavanja. Superskalarni hardver
sam otkriva zavisnosti u toku rada, pa **može** i bez pomoći kompajlera (ali mu pomoć koristi).

**Vrsta paralelizma koju VLIW eksploatiše:** paralelizam na nivou instrukcije (**ILP**).

---

# DEO 3 — Memorijski banci i vektorski korak

## 0. Zašto banke uopšte postoje

Latentnost memorije je velika u odnosu na brzinu procesora. Memorija se deli na **nezavisne
banke** kojima se pristupa **jednovremeno** — dok jedna banka „radi" svoju latentnost, zahtev
ide drugoj.

**Adresiranje: susedne adrese idu u RAZLIČITE banke** (niži bitovi adrese = broj banke). Zato
sekvencijalni pristup (korak 1) nikad ne pravi problem — svaki sledeći element je u drugoj
banci.

## Odakle konflikt

Kad je **vektorski korak ≠ 1**, pristupi preskaču banke i mogu se **vratiti na istu banku**
pre nego što je ona završila prethodni zahtev.

**Ključna veličina:** posle koliko pristupa se vraćaš na istu banku?
```
NZS(korak, n) / korak
```
`NZS(korak, n)` je prva adresa na kojoj se obrazac ponovi; podeljeno korakom daje **broj
pristupa** do povratka na istu banku.

## Uslov
```
NZS(korak, n) / korak  <  latentnost   ⟹  IMA konflikta
NZS(korak, n) / korak  ≥  latentnost   ⟹  NEMA konflikta
```
**Logika:** banka je zauzeta `latentnost` ciklusa. Ako se vratiš na nju **pre** nego što je
istekla → čekaš. Ako se vratiš **posle** → slobodna je.

Jednakost se tretira kao **nema konflikta** (banka baš tog trenutka završava).

## TIP: nađi najmanji broj banaka (stepen dvojke) bez konflikta

**Tok razmišljanja:**
1. Napiši uslov: `NZS(korak, n) ≥ latentnost · korak`
2. Probaj `n = 2, 4, 8, 16, 32…` dok ne prođe
3. Prvi koji prođe je odgovor

**Kako brzo računaš NZS(korak, n) kad je n stepen dvojke:**
- rastavi korak na `2^a · (neparan deo)`
- NZS(korak, n) = `korak · n / NZD(korak, n)`, a `NZD = 2^min(a, log₂n)`

**Brzi obrazac za NEPARAN korak:** NZD = 1 → `NZS = korak · n` → uslov postaje **n ≥
latentnost**. Zaokruži nagore na stepen dvojke i gotovo.

**Za PARAN korak brzi obrazac NE važi** — parni faktori u koraku „pojedu" deo n. Primer: korak
12 = 4·3, pa sve dok je n ≤ 4, `NZS(12,n) = 12` i odnos je **1** bez obzira na n — dodavanje
banaka ne pomaže dok n ne premaši parni faktor koraka.

**Sanity check:** odnos `NZS/korak` mora rasti kako povećavaš n. Ako ti ostaje isti — n je još
uvek manje od parnog faktora u koraku.

---

# Kontrolna lista

**Bernštajn**
- Jesi li napravio I i O za **ceo** proces (sve naredbe zajedno)?
- Testiraš li **svaki par** posebno?
- Kod grafa prethođenja — je li vreme = **kritični put**, a procesori = **najveća širina**?

**SP/VLIW**
- VLIW = samo statičko. Ako negde piše da VLIW planira dinamički — netačno.

**Banci**
- Je li korak paran? Onda brzi obrazac `n ≥ latentnost` **ne važi**
- Jesi li krenuo od najmanjeg stepena dvojke i išao naviše?
- Jednakost = **nema** konflikta
