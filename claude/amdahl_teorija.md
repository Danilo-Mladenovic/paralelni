# Amdahlov zakon — tipovi zadataka i kako se razmišlja

# 0. Odakle sve kreće

Normalizuj sekvencijalno vreme na **1**. Program se sastoji iz dva dela:

- **serijski deo, α** — izvršava se na jednom procesoru **bez obzira koliko ih imaš**.
  Ostaje **α**, nikad se ne smanjuje.
- **paralelni deo, (1−α)** — deli se na n procesora, pa traje **(1−α)/n**.

```
T(n) = α + (1−α)/n
        ↑        ↑
   ne deli se  deli se sa n
```

Ubrzanje je odnos starog i novog vremena:
```
S(n) = T(1)/T(n) = 1 / ( α + (1−α)/n )
```

**Zato u `1/(0.85 + 0.15/100)` nema magije:** 0.85 je serijski deo koji stoji sam za sebe,
a 0.15/100 je paralelni deo (15% programa) podeljen na 100 procesora.

**Efikasnost** = koliko je svaki procesor stvarno iskorišćen:
```
E(n) = S(n)/n         (E=1 znači idealno, svaki procesor radi punim kapacitetom)
```

**Maksimum:** pusti n→∞, član (1−α)/n nestaje:
```
S(∞) = 1/α
```
Ovo je **zid** — koliko god procesora dodaš, ne možeš ga probiti.

---

# 1. Kako iz teksta izvučeš α

**Najčešći izvor grešaka.** α je **UVEK ono što se NE paralelizuje**. Tekst ti može dati bilo
koju od dve strane:

| Formulacija u tekstu | α je |
|---|---|
| „15% je paralelizovano" | **0.85** (ostatak) |
| „90% se može paralelizovati" | **0.10** (ostatak) |
| „40% dela koji se ne može paralelizovati" | **0.40** (direktno) |
| „serijski deo traje 20s od ukupno 100s" | **20/100 = 0.20** |
| dat kod: petlja sa pragma + petlja bez pragma | udeo vremena **petlje bez pragma** |

**Provera zdravim razumom posle svakog računa:** S(n) mora ispasti **između 1 i 1/α**.
Ako dobiješ više od 1/α — pogrešio si α (verovatno uzeo paralelni udeo umesto serijskog).

---

# 2. Tipovi zadataka

## TIP 1 — dato α i n, traži se ubrzanje
*(najosnovniji; javlja se kao deo skoro svakog zadatka)*

**Tok razmišljanja:**
1. Izvuci α po tabeli iz odeljka 1
2. Izvuci n
3. `S = 1/(α + (1−α)/n)`
4. Ako traže i „maksimalno moguće" → `S(∞) = 1/α`

**Zamka:** ako u zadatku stoje **dva slučaja** sa različitim α i n, poenta nikad nije samo
uvrstiti brojeve — traži se **zaključak iz poređenja**. Uporedi:
- koliko je S daleko od svog **sopstvenog** maksimuma 1/α
- kolike su **efikasnosti** E = S/n

Tipičan konstruisani kontrast: mnogo procesora + mali paralelni udeo daje **manje** ubrzanje
nego malo procesora + veliki paralelni udeo. Zaključak koji se traži: **α je usko grlo, ne n**;
dodavanje procesora programu koji je već blizu 1/α je bacanje resursa.

*Javlja se: A1b, A6, A7, A12.*

---

## TIP 2 — traženo ubrzanje SAMO paralelnog dela

Formulacija: *„koliko bi trebalo da bude ubrzanje paralelnog dela da bi ubrzanje celog
programa bilo X?"*

**Kako razmišljaš:** ne ubrzava se ceo program za faktor Sp, nego **samo paralelni deo**.
Vrati se na izvođenje — paralelni deo umesto `(1−α)/n` sad traje `(1−α)/Sp`:
```
S_ukupno = 1 / ( α + (1−α)/Sp )
```
To je **ista formula**, samo Sp igra ulogu koju je igralo n. Reši po Sp:
```
Sp = (1−α) / (1/S_ukupno − α)
```

**Ključno zapažanje:** Sp i n su matematički ista stvar u formuli. „Ubrzati paralelni deo 6×"
i „pustiti paralelni deo na 6 procesora" daju isto ukupno ubrzanje.

**Prvo proveri izvodljivost:** ako je traženo S_ukupno ≥ 1/α, **nema rešenja** — imenilac
`1/S − α` postaje ≤ 0, pa bi Sp ispalo negativno ili beskonačno.

*Javlja se: A1a.*

---

## TIP 3 — ograničen broj jednakih delova (k delova, n procesora)

Formulacija: *„paralelni deo može biti podeljen na k jednakih delova"*, dato n procesora.

**Zašto obična formula NE važi.** Obična formula pretpostavlja da se paralelni deo deli
**proizvoljno fino** na n komada. Ovde je posao unapred izlomljen na **k nedeljivih komada**
— ne možeš dati procesoru pola komada.

**Kako razmišljaš — u „talasima":**
- Ako je n ≥ k → svaki komad dobija svoj procesor, sve ide **odjednom** = **1 talas**.
  Višak procesora preko k **ne pomaže ništa**.
- Ako je n < k → komadi se izvršavaju u **⌈k/n⌉ talasa**. Svaki talas traje koliko **jedan
  komad**, jer su svi komadi jednaki.

```
T(n) = α + ⌈k/n⌉ · (1−α)/k
             ↑        ↑
        broj talasa  trajanje jednog komada
```

**Zamka koju profesorka traži:** ⌈ ⌉ je **zaokruživanje NAGORE**. Sa k=6 i n=4 imaš
⌈6/4⌉ = **2** talasa, ne 1.5 — u drugom talasu rade samo 2 procesora, a 2 stoje besposlena,
ali talas ipak traje puno vreme jednog komada.

**Sanity check:** ako ti n deli k tačno (npr. k=6, n=3 → 2 talasa), rezultat mora biti isti
kao obična formula sa n. Ako ne deli — rezultat je **gori** od obične formule.

*Javlja se: A2, A3.*

---

## TIP 4 — data efikasnost, traži se α i/ili S

**Kako razmišljaš:** E i S su vezani preko `E = S/n`, pa je **S trivijalno**: `S = E·n`.
Za α — kreni od definicije efikasnosti i ubaci Amdahla:
```
E = S/n = 1/(n·α + (1−α))  =  1/(1 + α(n−1))
```
odakle:
```
α = (1/E − 1) / (n − 1)
```

**Šta ovde treba primetiti (ne samo izračunati):** kod velikog n, da bi E ostalo pristojno,
α mora biti **mikroskopsko**. Za n=1000 i E=80% ispada α ≈ 0.00025 — dakle svega **0.025%**
programa sme biti serijsko. To je poenta zadatka: **zahtev za visokom efikasnošću na velikom
broju procesora je brutalno strog prema serijskoj frakciji.**

*Javlja se: A4.*

---

## TIP 5 — da li je ubrzanje ostvarivo + minimalan broj procesora

Formulacija: *„da li je moguće postići ubrzanje S? Ako da, koliko je minimalno procesora
potrebno?"*

**Tok razmišljanja — dva koraka, i prvi se NE sme preskočiti:**

**Korak 1 — izvodljivost.** Izračunaj `S(∞) = 1/α`. Ako je traženo `S ≥ 1/α` → **nije moguće,
tačka**. Ne računaj dalje. (Ovo je pola poena.)

**Korak 2 — minimalno n.** Reši `S = 1/(α + (1−α)/n)` po n:
```
n = (1−α) / (1/S − α)
```
**Zaokruži NAGORE** — sa manje procesora ne postižeš traženo, a procesor ne postoji u pola
komada.

**Dodatna zamka:** zadatak ti često kaže „sistem se sastoji od 10 procesora" — pa proveri da
li dobijeno n **staje** u taj sistem. Ako ispadne n=8 a imaš 10 → u redu. Ako ispadne 15 →
odgovor je da na tom sistemu nije ostvarivo, iako je teorijski moguće.

*Javlja se: A5.*

---

## TIP 6 — više paralelnih delova sa različitim ubrzanjima

**Kako razmišljaš:** vrati se na izvođenje. Vreme je **zbir trajanja svih delova**; svaki deo
se skraćuje svojim sopstvenim faktorom:
```
T_novo = α_seq + α₁/S₁ + α₂/S₂ + …

S = 1 / ( α_seq + Σ αᵢ/Sᵢ )
```

**Provera:** svi udeli moraju sabrati na 1 (`α_seq + α₁ + α₂ + … = 1`). Ako ti tekst da samo
paralelne udele (npr. 30% i 40%), **serijski je ostatak** (30%).

**Zamka:** broj procesora u tekstu je često **irelevantan crveni haring** — ako su ubrzanja
pojedinih delova već data (S₁=3, S₂=4), podatak „4-procesorski sistem" ti ne treba ni za šta.
Ne pokušavaj da ga uguraš u formulu.

*Javlja se: A8.*

---

## TIP 7 — superlinearno ubrzanje i poređenje merenih podataka

Ovaj tip ima dva potpuno različita dela.

### 7a — „ubrzanje je veće od n, efikasnost veća od 1. Zašto?"

**Kako razmišljaš:** Amdahl kaže da je S ≤ n. Ako merenje pokazuje S > n, to **ne znači da je
fizika prekršena** — znači da **paralelna verzija radi MANJE efektivnog posla po procesoru**
nego što je sekvencijalna radila. Traži se uzrok te promene:

- **Efekat keša (najčešći odgovor):** kad se problem podeli, radni skup po procesoru je manji
  i **staje u lokalni keš**. Promašaji keša koji su mučili sekvencijalnu verziju nestaju, pa
  svaki procesor radi na bržoj memoriji. Nije isti posao — jeftiniji je.
- **Manje pretrage:** kod search/branch-and-bound algoritama paralelna verzija može „slučajno"
  naći rešenje ranije i **preskočiti grane** koje bi sekvencijalna morala da obradi.

**Šta se NE sme napisati:** „tako je ispalo" ili „paralelizam je efikasniji". Mora se imenovati
**mehanizam** koji je smanjio posao.

### 7b — dati izmereni S (ili α/e) za male n, proceniti ponašanje na velikom n

**Kako razmišljaš — ne gledaš S, gledaš da li je α KONSTANTNO.**

Iz svakog merenog para (S, n) izvuci eksperimentalno α:
```
α = (n/S − 1) / (n − 1)
```
Zatim pogledaj **trend**:

- **α konstantno preko svih merenja** → model je stabilan, Amdahl važi, `S(∞) = 1/α` je
  pouzdana granica. **Ekstrapolacija na veliko n je opravdana.**
- **α RASTE sa n** → overhead (komunikacija, sinhronizacija) raste sa brojem procesora.
  Amdahl sa fiksnim α **ne opisuje taj program**. Ekstrapolacija je **nepouzdana** — realnost
  će biti gora od bilo koje procene izvučene iz malih n.

**Zaključak koji se traži:** za veliki broj procesora bira se program sa **konstantnim α**,
**čak i ako izgleda lošije na malom n**. Predvidivost skaliranja je važnija od boljeg rezultata
na 4 ili 8 procesora.

*Javlja se: A9.*

---

## TIP 8 — poređenje dva algoritma (rad u flopsima / apsolutno vreme)

Formulacija: dva algoritma, jedan sa manje ukupnog posla ali slabo paralelizabilan, drugi sa
više posla ali dobro paralelizabilan. Koji izabrati za dati broj procesora?

**Kako razmišljaš — i gde svi padnu:** **NE porediš ubrzanja.** Ubrzanje je odnos prema
**sopstvenom** sekvencijalnom vremenu, pa algoritam sa 10× više posla može imati sjajno
ubrzanje i i dalje biti sporiji. **Porediš APSOLUTNO vreme izvršenja.**

```
za svaki algoritam:  T(n) = T_seq_tog_algoritma · ( α + (1−α)/n )
pa uporedi dobijene T(n)
```
Algoritam bez paralelizma ima T(n) = T_seq, nepromenjeno bez obzira na n.

**Šta zadatak zapravo ispituje:** da li razumeš da je ubrzanje **relativna** mera i da je
beskorisna za poređenje **različitih** algoritama. Odgovor uvek mora navesti oba vremena i
reći koje je manje.

*Javlja se: A10.*

---

## TIP 9 — analiza paralelnog algoritma sa komunikacijom

Formulacija: dat opis paralelnog algoritma (npr. traženje maksimuma na 4 procesora), cena X
ciklusa po slanju/prijemu elementa, Y ciklusa po poređenju. Traži se vreme i uslov
isplativosti.

**Kako razmišljaš — po fazama, i pitaš se za svaku „šta ide paralelno":**

1. Razloži algoritam na **faze** (raspodela podataka → lokalno računanje → spajanje rezultata).
2. Za svaku fazu izračunaj njeno trajanje, pri čemu **stvari koje se dešavaju istovremeno
   računaš samo jednom**. Ako dva para procesora razmenjuju podatke preko različitih veza,
   to je **jedan** X, ne dva.
3. Faza spajanja ide **stablasto**: 4 procesora → 2 nivoa (parovi, pa pobednici), 8 procesora
   → 3 nivoa. Broj nivoa je **log₂(broj procesora)**.
4. Saberi faze → T_par. Sekvencijalno je trivijalno (npr. N−1 poređenja → (N−1)·Y).
5. Uslov isplativosti: postavi **nejednačinu T_par < T_seq** i reši je po odnosu X i Y.

**Šta se traži kao odgovor:** ne samo broj, nego **odnos** — npr. „paralelni algoritam se
isplati dok je vreme slanja manje od otprilike 3× vremena poređenja". Zatim posmatraj šta
biva kad N→∞ (obično se granica ustali na konstantan odnos).

*Javlja se: A11.*

---

## TIP 10 — dat OpenMP kod, traži se ubrzanje

**Kako razmišljaš — ceo zadatak je zapravo „odredi α iz koda":**

1. **Pronađi šta ima `#pragma`, a šta nema.** Petlja bez pragme se izvršava **cela
   sekvencijalno** — to je tvoj serijski deo.
2. **Prebroj operacije u svakom delu** za veliko N (konstante se zanemaruju).
3. **Proveri balans grana.** Ako paralelna petlja ima if/else, prebroj operacije u **obe**
   grane:
   - iste → petlja je **balansirana**, deli se čisto sa n, grananje **ne smeta**
   - različite → postoji disbalans, deo niti čeka
4. α = (operacije u sekvencijalnom delu) / (ukupno operacija).
5. Uvrsti u običnu formulu.

**Zamka koja je cela poenta:** grananje `if (i % 16 < 8)` **izgleda** kao problem, ali ako obe
grane koštaju isto, ono ne utiče na ubrzanje ni najmanje. Studenti se zakuvaju u analizu
grananja i **previde da je prava serijska frakcija druga petlja** — ona bez pragme.

*Javlja se: A12.*

---

# 3. Rezime formula

| Traženo | Formula |
|---|---|
| Ubrzanje | S(n) = 1/(α + (1−α)/n) |
| Maksimum | S(∞) = 1/α |
| Efikasnost | E(n) = S(n)/n |
| α iz izmerenog S i n | α = (n/S − 1)/(n − 1) |
| n za traženo S | n = (1−α)/(1/S − α), **zaokruži nagore** |
| Ubrzanje samo paralelnog dela | Sp = (1−α)/(1/S_uk − α) |
| α iz E i n | α = (1/E − 1)/(n − 1) |
| k nedeljivih delova, n procesora | T(n) = α + ⌈k/n⌉·(1−α)/k |
| Više delova, različita ubrzanja | S = 1/(α_seq + Σ αᵢ/Sᵢ) |

# 4. Kontrolna lista pre nego što predaš odgovor

- Je li α ono što se **NE** paralelizuje? (najčešća greška)
- Je li S između 1 i 1/α?
- Ako ima nedeljivih delova — jesi li zaokružio broj talasa **nagore**?
- Ako se traži minimalno n — jesi li **prvo** proverio da je S < 1/α?
- Ako se porede dva algoritma — porediš li **vremena**, a ne ubrzanja?
- Ako se traži zaključak, a ne samo broj — jesi li napisao **zašto**, ne samo koliko?
