# Paralelni sistemi — teorijska pitanja sa blanketa

Izvučeno iz 23 blanketa sa 22 roka (svi su stara akreditacija). **Izostavljeni su MPI
zadaci, CUDA programerski zadaci i veliki OpenMP zadaci tipa "paralelizovati petlju +
testirati"** (popisani su na kraju, bez teksta). **Scoreboard i Tomasulo su izostavljeni**
po zahtevu — javljaju se samo na rokovima iz 2023, od 2024. naovamo ih nema.

Oznake rokova su **imena fajlova**, ne datumi sa papira.

## Rokovi

| Rok | Napomena |
|---|---|
| **2022_sep** | |
| **2022_dec** | |
| **2023_jan** | |
| **2023_kol1** | I kolokvijum |
| **2023_kol1-popravni** | I kolokvijum, popravni |
| **2023_jun** | |
| **2023_jun2** | |
| **2023_sep** | |
| **2023_okt** | |
| **2023_dec** | |
| **2024_jan** | |
| **2024_kol1** | I kolokvijum |
| **2024_jun** | |
| **2024_okt** | |
| **2025_jan** | |
| **2025_apr** | |
| **2025_jun** | |
| **2025_sep** | |
| **2025_okt** | |
| **2025_okt2** | |
| **2026_jan** | |
| **2026_apr** | |

---

# A. Amdahlov zakon, ubrzanje, efikasnost

**A1. [2024_jun]** 90% nekog programa se može paralelizovati.
a) Koliko bi trebalo da bude ubrzanje paralelnog dela programa da bi ubrzanje celog
programa bilo 4x u odnosu na sekvencijalno izvršenje?
b) Koliko je maksimalno moguće ubrzanje za ovaj program?

**A2. [2024_kol1 / 2026_apr]** Na raspolaganju je 4-jezgarni procesor koji može izvršavati 4
procesa jednovremeno (po jedan proces na svakom jezgru). Program ima 40% dela koji se ne
može paralelizovati, dok ostatak programa može biti podeljen na 6 jednakih delova koji se
mogu izvršavati paralelno. Koliko se maksimalno ubrzanje može ostvariti kod izvršenja ovog
programa na 4-jezgarnom procesoru?

**A3. [2025_sep]** *(varijanta A2)* …Koliko se maksimalno ubrzanje može ostvariti na
4-jezgarnom procesoru? Koliko je max ubrzanje ako imamo 10 procesora na raspolaganju?

**A4. [2025_jan]** Ako imamo računar sa 1000 procesora, koliko mora biti ubrzanje i serijska
frakcija da bi efikasnost bila 80%?

**A5. [2022_sep]** Sistem se sastoji od 10 procesora. 98% koda se može paralelizovati. Da li
je moguće na ovom sistemu postići ubrzanje 7? Ako da, koliko je minimalno procesora
potrebno da bi se to postiglo?

**A6. [2023_sep]** Ako imamo 100 procesora i program koji ima 15% dela koji je paralelizovan,
koliko je ubrzanje takvog sistema? Ako imamo 10 procesora i program koji ima 85% dela koji
je paralelizovan, koliko je ubrzanje takvog sistema?

**A7. [2023_kol1 / 2023_kol1-popravni]** Program P se izvršava za 100 sec na jednoprocesorskom sistemu.
Kada se program paralelizuje, 20 sec traje izvršenje dela programa koji se ne može
paralelizovati, a ostali deo programa se može idealno paralelizovati.
a) Ako se program izvršava na 16-procesorskom sistemu, koliko vremena će trajati izvršenje
programa?
b) Ako imamo beskonačno mnogo procesora na raspolaganju, koliko vremena će trajati
izvršenje programa?

**A8. [2026_jan]** Program ima dve paralelne porcije (dela) i jednu sekvencijalnu. Dve
paralelne porcije čine, redom, 30% i 40% programa. Ako se program izvršava na
4-procesorskom sistemu, ubrzanje dva paralelna dela iznosi, redom, 3 i 4. Koliko je ukupno
ubrzanje programa?

**A9. [2025_apr]**
a) Ubrzanje 4-procesorskog sistema je veće od 4 i efikasnost je veća od 1. Količina
izračunavanja je ista kao kod sekvencijalne verzije programa. Šta je razlog ovog
super-linearnog ubrzanja?
b) Treba da odlučite koji od dva programa da koristite. Program će se izvršavati na
512-procesorskom sistemu, ali trenutno na raspolaganju imate svega 16 procesora za merenje
performansi. S = ubrzanje, e = eksperimentalno utvrđena serijska frakcija, p = broj procesora.
Koji program će dati bolje performanse na 512-procesorskom sistemu? Kratko objašnjenje.

| Program A | p | 2 | 4 | 8 | 16 |
|---|---|---|---|---|---|
| | S | 1.88 | 3.12 | 4.5 | 5.8 |
| | e | 0.06 | 0.09 | 0.11 | 0.12 |

| Program B | p | 2 | 4 | 8 | 16 |
|---|---|---|---|---|---|
| | S | 1.79 | 2.9 | 4.3 | 5.7 |
| | e | 0.15 | 0.15 | 0.15 | 0.15 |

**A10. [2025_okt]** Imate dva algoritma koja možete izabrati za rešavanje zadatog problema.
Jedan radi samo serijski, ne može se paralelizovati i potrebno mu je 2,0 teraflopsa da reši
problem. Drugi može biti paralelizovan, ali samo 2/3 od serijskog vremena izvršavanja može
biti paralelizovano. Ukupno, ovaj algoritam zahteva 3,0 teraflopsa kada se sekvencijalno
izvršava. Na raspolaganju je 4-procesorski sistem. Koji algoritam biste odabrali? Ignorišite
bilo kakvo komunikaciono kašnjenje.

**A11. [2024_okt / 2023_okt / 2025_okt2]** Da bi se pronašao maksimalni element u vektoru od N
elemenata na četvoroprocesorskom sistemu može se iskoristiti sledeći algoritam. Inicijalno
se svi elementi vektora nalaze u procesoru P0:
1. P0 šalje po N/4 elemenata svakom od procesora.
2. Svi procesori pronalaze maksimalni element u svom delu vektora.
3. Procesori P0 i P1 razmenjuju nađene maksimalne vrednosti da bi odredili veću. U isto
   vreme to rade i procesori P2 i P3.
4. P0 prima pronađeni maksimalni element od procesora P2 i određuje maksimalni element u
   celom vektoru.

Druga mogućnost je da se pronalaženje maksimalnog elementa odredi sekvencijalno. Ako
usvojimo da je potrebno X ciklusa za send/receive jednog elementa između procesora, i Y
ciklusa da se uporede dva elementa, koliko je ciklusa potrebno za izvršenje sekvencijalnog,
a koliko za izvršenje paralelnog programa? Pod kojim uslovom će paralelni algoritam biti
bolji od sekvencijalnog?

**A12. [2023_jan / 2025_apr]** Sledeći OpenMP program se izvršava na 4-jezgarnom procesoru.
Koliko se ubrzanje može postići za veliko N? Vreme potrebno za testiranje u if naredbi
zanemariti. Usvojiti da ostale operacije traju 1 vremensku jedinicu.
```c
float total = 0.0;
#pragma omp parallel for
for (int i=0; i<N; i++) {
    if (i % 16 < 8)   // usvojiti da testiranje zahteva 0 operacija
        out[i] = 1 * in[i];   // 1 op
    else
        out[i] = 2 + in[i];   // 1 op
}
for (int i=0; i<N; i++)
    total += out[i];   // 1 op
```

---

# B. Bernštajnovi uslovi

**B1. [2024_kol1]** Korišćenjem Bernštajnovih uslova utvrditi koji se procesi mogu izvršavati
paralelno:
```
P1: C = D x E        P2: A = B + C        P3: F = G / E
    M = G + C            C = L + M
```

**B2. [2023_jun]** Korišćenjem Bernštajnovih uslova utvrditi koje naredbe se mogu izvršavati
paralelno:
```
P1: C = D x E
P2: M = G + C
P3: A = B + C
```

**B3. [2023_jun2]** Korišćenjem Bernštajnovih uslova utvrditi koji od procesa, P1, P2 i P3, se
mogu izvršavati paralelno:
```
P1: C = D x E        P2: C = L + M        P3: F = B * D
    M = G + C            F = G / E            E = F / E
    A = B + C
```

**B4. [2025_jun]** Korišćenjem Bernštajnovih uslova odrediti maksimalni paralelizam između
instrukcija u sledećem segmentu:
```
S1: X = Y + Z
S2: Z = U + V
S3: R = S + V
S4: Z = X + R
S5: Q = M + Z
```
Ako se svaka od instrukcija izvršava za 1 vremensku jedinicu, za koliko vremena će se
izvršiti paralelna verzija programa? Koliko je ubrzanje u tom slučaju? Koliko se maksimalno
ubrzanje može postići paralelnim izvršenjem navedenog kodnog segmenta?

---

# C. Analiza zavisnosti u petljama

## C.1 ZIV / SIV / MIV subscripti

**C1. [2023_jun2 / 2024_jun]**
a) U sledećem primeru identifikovati sve ZIV, SIV i MIV subscripte i označiti koji su
separabilni a koji povezani.
```
for i=1,N
  for j=1,N
    for k=1,N
      A(2i+1, 5, j) = A(2i-2, N, k) + B(i, j+i, k)
```
b) Na osnovu analize subscripta utvrditi da li postoji loop carry zavisnost u navedenom
primeru, bez korišćenja vektora/pravaca zavisnosti.

**C2. [2025_jun]**
a) U sledećem primeru identifikovati sve ZIV, SIV i MIV subscripte.
```
for j = 1, 10
  for i = 1, 99
  {
    A(i,j) = B(j) + X;
    B(j+1) = A(100-i, j-1) + Y;
  }
```
b) Odrediti sve vektore i pravce zavisnosti. Da li se unutrašnja petlja može vektorizovati?

## C.2 GCD test

> Formulacija je uvek ista: *"Objasniti GCD test za detekciju loop-carry zavisnosti. Pomoću
> GCD testa utvrditi da li u sledećem kodu postoji loop-carry zavisnost."* Menja se samo kod.

**C3. [2024_kol1]**
```
for p = 1, P; for q = 1, Q; for r = 1, R; for s = 1, S {
  A(2p + 2q + 3s) = ...
                  = A(p - 6q + 5r + 3)
}
```

**C4. [2025_apr]**
```
for p = 1, P; for q = 1, Q; for r = 1, R; for s = 1, S
  A(2p + 2q - r + 3s) = A(p - 6q + 5r + 3) * B(r+s-4q) + A(p-r+2s+q+2)
```

**C5. [2023_kol1-popravni / 2023_jun]**
```
for p = 1, P; for q = 1, Q; for r = 1, R; for s = 1, S
  A(2p + 2q - r + 3s) = A(p - 6q + 5r + 3) * B(r+s-4q)
```

**C6. [2022_sep]**
```
for i := 1, n1
  for j := 1, n2
    for k := 1, n3
      a(2*i + 5*j + 4*k) = a(6*i + 2*j - 7*k + 4) + …
```

**C7. [2026_jan]**
```
for (i=0; i<p; i++)
  for (j=1; j<q; j++)
    for (k=1; k<r; k++)
      a(4*i + 2*j + 1) = … A(6*i + 2*k + 4);
```

## C.3 Vektori i pravci zavisnosti, transformacije, vektorizacija

**C8. [2024_kol1]** Odrediti sve vektore i pravce zavisnosti u sledećem gnezdu petlji. Da li se
unutrašnja petlja može vektorizovati?
```
for j = 1, 10
  for i = 1, 99
  {
    A(i,j) = B(j) + X;
    B(j+1) = A(100-i, j-1) + Y;
  }
```

**C9. [2024_okt]** Da li je na sledećem gnezdu petlji
```
for i=1,n
  for j=1,n
    c(i,j) = c(i+1, j-1) + 1
  endfor {i,j}
```
dozvoljeno primeniti: a) permutaciju petlji i i j, b) obrtanje po indeksnoj promenljivoj j,
c) kompoziciju transformacija pod a. i b.? Svaki odgovor obrazložiti. U slučaju da je
dozvoljeno izvršiti navedenu transformaciju, prikazati kako izgleda transformisano gnezdo
petlji.

**C10. [2025_okt2]** Da li je u sledećem primeru moguće obaviti vektorizaciju unutrašnje
petlje? Obrazložiti odgovor. U slučaju da je odgovor negativan, transformisati petlje tako
da vektorizacija bude moguća.
```
for (j = 0; j < N; j++)
  for (i = 1; i < N; i++)
    A[i+1, j-2] = A[i, j] + A[i-1, j-2];
```

**C11. [2023_kol1 / 2023_kol1-popravni / 2025_okt]**
a) Odrediti sve vektore zavisnosti i pravce zavisnosti za zadato gnezdo petlji.
b) Za svaku od petlji odgovoriti da li se može paralelizovati/vektorizovati, i ako ne može
zbog kojih zavisnosti ne može.
c) Primeniti odgovarajuću transformaciju tako da se unutrašnja petlja može vektorizovati.
Pokazati ceo postupak kako se dolazi do transformisane petlje.
```
for (i=2; i<N-1; i++) {      // loop1
  for (j=3; j<N; j++) {      // loop2
    for (k=4; k<N-3; k++) {  // loop3
      A[i][j][k] = A[2i-4][j-3][k-4] + A[i+1][j][k+2] + A[i][j][k-1];
    }}}
```

**C12. [2023_dec]** Po kojoj indeksnoj promenljivoj je moguće izvršiti vektorizaciju sledećeg
gnezda petlji? Pokazati kako izgleda transformisano gnezdo petlji u kome se unutrašnja
petlja može vektorizovati.
```
for (k = 0; k < L; ++k)
{
  for (j = 0; j < M; ++j)
  {
    for (i = 0; i < N; ++i)
    {
      a[i][j][k+1]     = a[i][j][k] + X1;
      b[i+1][j][k]     = b[i][j][k] + X2;
      c[i+1][j+1][k+1] = c[i][j][k] + X3;
    }
  }
}
```

**C13. [2025_sep / 2026_apr]** Da li se sledeća petlja može vektorizovati:
```
for (i = 1; i<N; i++)
  A(i) = A(N-i+1) + C;
```
Ako je odgovor negativan, transformisati petlju tako da se može vektorizovati.

**C14. [2025_jan]** Da li je sledeće gnezdo petlji moguće paralelizovati po I?
```
for (I=0; I<100; I++)
  for (J=1; J<100; J++)
    B(J) = B(J-1) + I;
```

## C.4 Eliminacija zavisnosti preuređenjem koda

**C15. [2023_jan]** Mnoge zavisnosti u programu mogu biti eliminisane preuređenjem koda.
Zadata je sledeća petlja:
```
for (i = 0; i < N; ++i)
{
S1:  a[i] = b[i] + C;
S2:  d[i] = a[i] + E;
}
```
Da li je korektno izvršiti preuređenje ove petlje na sledeći način (obavezno obrazložiti
odgovor):
```
d[0] = a[0] + E;
for (i = 1; i < N; ++i)
{
S1:  a[i-1] = b[i-1] + C;
S2:  d[i]   = a[i] + E;
}
a[n] = b[n] + C;
```

**C16. [2025_okt2]** Objasniti WAR zavisnost po podacima. Napisati primer za petlju u kojoj
postoji ova zavisnost i jasno je označiti.

**C17. [2026_apr]** Da li se sledeća petlja može paralelizovati? Ako može, paralelizovati je
pomoću OpenMP, ako ne objasniti zašto ne može.
```c
for (i=1; i<n-1; i++) {
  a[i]   = b[i+1] * c;
  a[i+1] = b[i]   * d;
}
```

---

# D. Sprežne mreže

## D.1 Višestepene dinamičke mreže

**D1. [2023_jan / 2023_dec / 2024_jun]** 32 PE povezana su hiperkub višestepenom spregnom mrežom.
a) Koliko stepena ima mreža i koliko komutacionih elemenata?
b) Iz PE-a sa adresom 21 potrebno je poslati poruku PE-ovima sa adresama 9, 13, 25 i 29.
Da li je moguće obaviti emisiju? Ako jeste kako izgleda zaglavlje koje se dodaje poruci?

**D2. [2025_jun]** U ESC (Extra Stage Cube) višestepenoj spregnoj mreži koja povezuje 32 PE
potrebno je poslati poruku iz čvora sa adresom 18 u čvorove sa adresama 4, 6, 12 i 14
(obaviti emisiju). Kako izgleda zaglavlje poruke za primarni i sekundarni put?

**D3. [2023_jun]**
a) U ESC (Extra Stage Cube) višestepenoj spregnoj mreži koja povezuje 16 PE-ova potrebno je
poslati poruku iz čvora sa adresom 3 u čvorove sa adresama 4 i 6 (obaviti emisiju). Kako
izgleda zaglavlje poruke za primarni i sekundarni put?
b) U hiperkub statičkoj spregnoj mreži sa 16 procesora potrebno je poslati poruku iz čvora
sa adresom 1001 u čvor 0110. Preko kojih čvorova će poruka biti prosleđena do odredišnog
čvora? Prikazati kako se došlo do rešenja.

**D4. [2025_okt]** Kojim sprežnim funkcijama su definisane sledeće dinamičke sprežne mreže:
a) Mešanje-zamena (shuffle-exchange), b) kub, c) rešetka (mesh)?

**D5. [2025_apr]** Dati primer sprežne mreže kod koje stepen čvora raste logaritamski sa
porastom broja čvorova. Skicirati mrežu sa 16 čvorova.

## D.2 Statičke mreže, topologija, izbor mreže

**D6. [2026_apr]** Koliko direktnih suseda ima čvor u 4D hiperkubu? Preko kojih čvorova se
stiže iz čvora sa adresom 2 do čvora sa adresom 13?

**D7. [2022_sep]** Potrebno je povezati 625 procesora. Na raspolaganju su vam sledeće sprežne
mreže: a) magistrala, b) potpuno povezana mreža, i c) 25x25 rešetka (mesh). Za svaku od
sprežnih mreža navesti po jednu prednost i nedostatak. Koju sprežnu mrežu biste odabrali?
Obrazložiti odgovor.

**D8. [2026_jan]** Razmotrimo multiprocesorski sistem sa 3 procesora P1, P2 i P3.
Pretpostavimo da su povezani ili zajedničkom magistralom, ili linearnom vezom ili prstenom.
U trenutku 1, sva tri procesora žele da pošalju poruku: P1 treba da pošalje poruku ka P2,
P2 ka P3 i P3 ka P1. Pretpostavimo da je svaka veza iste brzine. Koliko dugo traje ova
komunikacija za svaku od mreža i zašto?

**D9. [2023_jan]** Objasniti arbitražu na magistrali koja se zasniva na deljivim zahtevima i
lančanom zahvatanju.

## D.3 SIMD maskiranje

**D10. [2025_okt]** U SIMD sistemu postoji 16 procesnih elemenata (PE). PE-ovima sa adresama
0, 1, 3, 9 i 12 treba dozvoliti rad a ostalima zabraniti. Kako izgledaju maske kojima se
ovo postiže ako se koristi direktno maskiranje a kako ako se koriste adresne maske?

**D11. [2023_jan]** Kako se može obaviti maskiranje procesnih elemenata u SIMD?

---

# E. Keš koherencija

## E.1 Konceptualna pitanja: snoopy vs. direktorijumske šeme, MIMD

**E1. [2023_sep]** Koja je najvažnija razlika između snoopy protokola i protokola baziranih na
direktorijumskim šemama?

**E2. [2023_okt]** Navesti bar jednu prednost i jedan nedostatak protokola baziranih na
direktorijumskim šemama u odnosu na snoopy protokole.

**E3. [2025_okt]** U čemu je razlika između čvrsto spregnutih i slabo spregnutih MIMD sistema?
Pretpostavimo da projektujete slabo spregnuti MIMD sistem sa 1024 procesora. Koju vrstu keš
koherentnog protokola biste koristili? Zašto?

## E.2 Write-once i Firefly — stanja, komande, akcije

**E4. [2023_jun2 / 2024_okt / 2026_apr]** Koja stanja keš kopije postoje kod Write-once a koja kod
Firefly protokola? Koje komande konzistencije se koriste kod Write-once a koje kod Firefly
protokola? Koje aktivnosti se dešavaju kod ovih protokola kada nastupi događaj **promašaj
pri upisu**?

**E5. [2022_sep]** Ako nastupi promašaj pri upisu kod obraćanja lokalnom kešu koje se akcije
preduzimaju i koje je rezultujuće stanje **lokalnih i udaljenih** kopija ako se koristi:
a) Write-once protokol, b) Firefly protokol. Objasniti koje se lokalne procesorske komande
i komande konzistencije tom prilikom emituju.

## E.3 Direktorijumske šeme — proračun bitova

**E6. [2024_jun]** Multiprocesorski sistem se sastoji od 256 procesora i koristi privatne keš
memorije. Veličina glavne memorije je 8GB, a veličina keš bloka je 64B.
a) direktorijumska šema sa potpuno preslikanim adresama — kolika je veličina direktorijuma
u glavnoj memoriji?
b) šema sa ograničenim adresama, maksimalno 4 procesora mogu imati kopiju jednog bloka —
kolika je veličina direktorijuma?
c) šema sa ulančanim adresama — kolika je veličina direktorijuma?

**E7. [2025_okt2]** Multiprocesorski sistem se sastoji od 128 procesora i koristi privatne keš
memorije. Veličina glavne memorije je 1 MB, a veličina keš bloka je 64B. Koliko ukupno
bitova stanja zahteva implementacija protokola za: a) potpuno preslikane adrese,
b) ograničene adrese (max 4 kopije), c) ulančane adrese?

**E8. [2025_apr]** Multiprocesorski sistem sa 256 procesora. Svaki procesor ima lokalnu keš
memoriju kapaciteta 1 MB. Kapacitet glavne memorije je 1 GB. Veličina keš bloka je 64 B.
a) write-once protokol — koliko ukupno bitova stanja zahteva implementacija?
b) direktorijumska šema sa potpuno preslikanim adresama — koliko ukupno bitova stanja?
c) Koju od ove dve šeme biste iskoristili? Obrazložiti odgovor.

**E9. [2023_jun]** Multiprocesorski sistem od 256 procesora koristi direktorijumsku šemu sa
**ulančanim adresama** za održavanje keš koherencije. Svaki procesor ima privatnu keš
memoriju kapaciteta 1MB, a veličina keš bloka je 64 byte. Veličina glavne memorije je 4 GB.
Koliko ukupno bitova stanja zahteva implementacija ovog protokola?

## E.4 Tabele — popunjavanje stanja

**E10. [2025_jan / 2025_jun]** U multiprocesorskom sistemu se za održavanje keš koherencije
koristi Write-once protokol. Inicijalno je keš memorija procesora P1 prazna. Popuniti tabelu
koja prikazuje promene stanja keš bloka u procesoru P1 u zavisnosti od komandi koje se izdaju.

| Događaj/komanda | Stanje keš bloka u P1 | Akcija keš kontrolera P1 (napisati ništa ako nema akcije) |
|---|---|---|
| P_Read A | | |
| P_Write A | | |
| Read_Blk A | | |
| Write_Inv A | | |
| P_Read A | | |
| P_Write A | | |
| Read_Blk A | | |
| P_write A | | |
| P_write A | | |

*(Varijanta 2025_jun: 5. i 6. red obrnuti — `P_Write A`, `P_Read A`.)*

**E11. [2024_okt / 2026_apr]** Multiprocesorski sistem sa 4 procesora koristi write-once
protokol. Inicijalno su keš memorije svih procesora prazne. Promenljive x i y se nalaze u
istom keš bloku. U tabeli staviti simbol "*" kada u određenom procesoru dolazi do promašaja
kod pristupa kešu.

| # | P0 | P1 | P2 | P3 |
|---|---|---|---|---|
| 1 | | Store X | | |
| 2 | | Load X | | |
| 3 | | | Store Y | |
| 4 | | | Load X | |
| 5 | | | | Load X |
| 6 | Load X | | | |
| 7 | | Load X | | |
| 8 | | | Store Y | |
| 9 | | Store X | | |
| 10 | | | Load Y | |
| 11 | | Load X | | |
| 12 | | Load Y | | |
| 13 | | | | Load X |
| 14 | | | | Store Y |

**E12. [2025_sep]** Sledeći kod se izvršava na dvoprocesorskom sistemu. Za održavanje keš
koherencije koristi se write-once protokol:
```
(1) LW x1, 0(x5)
(2) LW x2, 0(x6)
(3) SW x3, 0(x6)
(4) SW x2, 0(x5)
(5) LW x1, 0(x6)
```
Adrese x5 i x6 se **ne** preslikavaju u isti keš blok. Instrukcije (1) do (5) se izvršavaju
od strane procesora A i B u redosledu: A.1, A.2, B.1, B.2, A.3, B.3, B.4, A.4, A.5, B.5.
Popuniti tabelu (za svaki procesor: stanje x5 linije, stanje x6 linije, komanda keš
kontrolera).

**E13. [2026_jan]** Dvoprocesorski sistem koristi Write once protokol. Promenljive X i Y se
preslikavaju u **isti** keš blok. Inicijalno X=0 i Y=0. Popuniti tabelu za sledeći scenario:

| Akcija | Stanje keša C1 | Stanje keša C2 | Komanda konzistencije | Vrednosti X i Y u GM |
|---|---|---|---|---|
| P1: X=1 | | | | |
| P2: Y=1 | | | | |
| P1: X=2 | | | | |

**E14. [2023_dec]** Multiprocesorski sistem se sastoji od tri procesora P1, P2 i P3. Deljiva
memorija se sastoji od 4 bloka x, y, z, w. Svaki procesor ima keš u kome se može naći samo
jedan blok u datom trenutku. Za postizanje keš koherencije koristi se Write-once protokol.
Keš memorije su inicijalno prazne, sadržaji memorijskih blokova su:

| Memorijski blok | x | y | z | w |
|---|---|---|---|---|
| sadržaj | 10 | 30 | 80 | 20 |

Prikazati sadržaje keševa i glavne memorije i stanje keš blokova i GM nakon svake operacije:

| akcija | keš P1 | keš P2 | keš P3 | glavna mem |
|---|---|---|---|---|
| P1 read(x) | | | | |
| P2 read(x) | | | | |
| P3 read(x) | | | | |
| P1 x=x+25 | | | | |
| P1 read(z) | | | | |
| P2 read(x) | | | | |
| P3 x=15 | | | | |
| P1 z=z+1 | | | | |

**E15. [2023_jan]** Multiprocesorski sistem se sastoji od 3 procesora. Svaki procesor ima
svoju keš memoriju. Prikazati kako se vrši promena bitova stanja u keš memorijama procesora
i u direktorijumu glavne memorije ako se za održavanje keš koherencije koriste **ograničeni
adresari** a maksimalni broj jednovremenih kopija je **2**. Procesori pristupaju deljivoj
promenljivoj X.

| Akcija procesora | Direktorijum u GM | Bitovi stanja u P1 | Bitovi stanja u P2 | Bitovi stanja u P3 |
|---|---|---|---|---|
| P1 čita X | | | | |
| P3 čita X | | | | |
| P3 upisuje u X | | | | |
| P1 čita X | | | | |
| P2 čita X | | | | |

---

# F. Superskalarni / VLIW procesori

**F1. [2023_kol1 / 2023_kol1-popravni / 2024_kol1]** Zaokružiti šta od navedenog važi za Superskalarne (SP), a
šta za VLIW procesore. Planiranje izvršenja instrukcija se obavlja:
- statički uz pomoć kompajlera (SP, VLIW)
- dinamički u fazi izvršenja programa (SP, VLIW)
- moguće su obe varijante (SP, VLIW)

---

# G. Vektorski procesori / memorijski banci

> Formulacija identična svaki put: *"Ako je latentnost memorije X clk ciklusa a vektorski
> korak Y, koliko je najmanje memorijskih banaka potrebno da bi se elementima vektora
> pristupilo bez konflikata ako broj banaka mora biti stepen dvojke?"*

| Oznaka | Rok | Latentnost | Vektorski korak |
|---|---|---|---|
| **G1** | 2024_kol1 | 6 clk | 12 |
| **G2** | 2025_okt2 | 8 clk | 12 |
| **G3** | 2023_kol1 / 2023_kol1-popravni | 6 clk | 10 |

---

# H. OpenMP — kratka teorijska pitanja

## H.1 Trka podataka i korektnost

**H1. [2023_sep / 2024_jun]** Deljiva promenljiva `a` je inicijalno postavljena na 0 (a=0). Dve
niti paralelno izvršavaju sledeće:
```
Thread 1          Thread 2
a++;              a++;
while(a < 2);     while(a < 2);
```
Koja od sledećih tvrdnji je tačna? Obavezno obrazložiti odgovor.
a) Obe niti nikada neće okončati izvršenje
b) Obe niti će uvek okončati izvršenje
c) Bar jedna od niti će uvek okončati izvršenje
d) Ili će obe niti okončati izvršenje, ili ni jedna neće okončati izvršenje

**H2. [2023_jan / 2024_okt]** Da li je sledeći OpenMP program korektan / šta je pogrešno u njemu?
Obavezno obrazložiti odgovor.
```c
void ccode (int n)
{
  #pragma omp parallel
  {
    #pragma omp critical
    {
      work1();
      #pragma omp barrier
      work2();
    }
  }
}
```

**H3. [2025_jan]** Zadat je sledeći OpenMP program:
```c
#pragma omp parallel for private(t, j)
for (i=0; i < 1000000; i++) {
  t = get_thread_id();
  for (j=0; j < 1000000; j++) {
    data[j] = t;
  }
}
```
Šta je od sledećeg tačno ako se kreiraju četiri niti:
a) u programu ne postoji trka podataka (data race)
b) u programu postoji trka podataka
c) elementi polja data će imati vrednost 0, 1, 2 ili 3
d) elementi polja data mogu imati bilo koju vrednost

**H4. [2022_sep]** Zadat je sledeći OpenMP program:
```c
int i, j, n = 100000, t = 0;
#pragma omp parallel for
for (i=0; i < n; i++) {
  t = t + 1;
}
printf("t = %d\n", t);
```
Koje od sledećih tvrđenja je tačno:
a) uvek će biti odštampana vrednost 100000
b) odštampana vrednost će biti 100000, a moguće su i vrednosti manje od 100000
c) bilo koja vrednost između -MAXINT i MAXINT je moguća
Kratko obrazložiti odgovor.

**H5. [2023_sep / 2025_okt2]** Pretpostavimo da imamo sledeći OpenMP program:
```c
main() {
  int x, y, z;
  ... // izračunavanja sa x, y i z
  #pragma omp parallel
  { ... // threads }
}
```
pri čemu promenljivoj `x` pristupa samo nit 1, promenljivoj `y` samo nit 2, a promenljivoj
`z` samo nit 3. Pretpostavimo da kompajler smesti promenljive x, y i z u susedne memorijske
lokacije. Navesti jedan razlog zašto bi ovakvo smeštanje podataka moglo loše uticati na
performanse programa.

**H6. [2026_apr]** Petar je paralelizovao program na sledeći način:
```c
#pragma omp parallel for
for (i = 0; i < n; i++) {
  #pragma omp critical
  sum = sum + data[i];
}
```
i nije postigao nikakvo ubrzanje. Šta je uzrok? Paralelizovati ovaj program bez korišćenja
reduction odredbe.

## H.2 Predviđanje izlaza

**H7. [2022_sep]** Zadat je sledeći OpenMP program. Koliko puta će biti odštampano `a`?
Koliko puta će biti odštampano `b`?
```c
int i, j, n = 10, t = 0;
omp_set_num_threads(2);
#pragma omp parallel private(j)
{
  #pragma omp for
  for (i=0; i < n; i++) { printf("a\n"); }
  for (j=0; j < n; j++) { printf("b\n"); }
}
```

**H8. [2023_sep]** Zadat je sledeći OpenMP program. a) Koliko puta će biti odštampano a?
b) Koliko puta će biti odštampano b? c) Koliko puta će biti odštampano c?
```c
int main (int argc, char *argv[]) {
  int i;
  omp_set_num_threads(4);
  #pragma omp parallel
  for (i=0; i < 10000; i++) {
    #pragma omp critical
    printf("a\n");
  }
  #pragma omp parallel
  for (i=0; i < 10000; i++) {
    #pragma omp master
    printf("b\n");
  }
  #pragma omp parallel for
  for (i=0; i < 10000; i++) {
    #pragma omp critical
    printf("c\n");
  }
}
```

**H9. [2023_jun]** Koja vrednost promenljive q će biti odštampana u sledećem programu?
Obavezno obrazložiti odgovor.
```c
void main() {
  int q = 2;
  #pragma omp parallel num_threads(4) reduction(*:q)
  {
    q += 2;
  }
  printf("%d\n", q);
}
```

**H10. [2025_okt]** Napišite izlaz iz sledećeg OpenMP koda ako je moguće, ili navedite grešku
ako postoji. Kreirane su 4 niti.

| A) | B) | C) |
|---|---|---|
| `int i;`<br>`double sum = 0.0;`<br>`#pragma omp parallel private(sum)`<br>`{`<br>`for (i=1; i <= 4; i++)`<br>`  sum = sum + 1;`<br>`}`<br>`printf("The sum is %lf\n", sum);` | `int i;`<br>`double sum = 0.0;`<br>`#pragma omp parallel shared(sum)`<br>`{`<br>`for (i=1; i <= 4; i++)`<br>`  sum = sum + i;`<br>`}`<br>`printf("The sum is %lf\n", sum);` | `int i;`<br>`double sum = 0.0;`<br>`#pragma omp parallel for reduction(+:sum)`<br>`for (i = 1; i <= 4; i++)`<br>`  sum = sum + i;`<br>`printf("The sum is %lf\n", sum);` |

**H11. [2025_sep]** Zadat je sledeći OpenMP program:
```c
#pragma omp parallel for private(i) num_threads(4) schedule(static, 10)
for (int i = 0; i < 100; i++)
  a[i] = i;
```
Koja nit će izvršiti 76. iteraciju? Koje iteracije će izvršiti nit 2?

## H.3 Scheduling i barijere

**H12. [2023_jun]** Na raspolaganju je multiprocesorski sistem sa 4 jezgra. Za paralelizaciju
for petlje koja inicijalizuje na nulu gornji trougao kvadratne matrice dimenzija 100×100
koristi se OpenMP:
```c
#pragma omp parallel for private(j) schedule( ... )
for (i = 0; i < 99; i++)
  for (j = i+1; j < 100; j++)
    a[i][j] = 0.0;
```
Poređati sledeće schedule odredbe počev od one koja će dati najbrže izvršenje do najsporije:
`schedule(static)`, `schedule(static, 10)`, `schedule(static, 1)`,
`schedule(dynamic, 1)`, `schedule(dynamic, 10)`, `schedule(dynamic, 20)`

**H13. [2026_jan]** Sledeća pitanja se odnose na programe označene sa Code 1A i Code 1B.

Koja schedule odredba se treba upotrebiti u **Code 1A** da bi se postigao najmanji disbalans
u opterećenju? Nemojte uzimati u obzir cenu planiranja (scheduling). Dati kratko obrazloženje.
Koja schedule odredba se treba upotrebiti u **Code 1B** da bi se postigao dobar balans
opterećenja i mala cena planiranja (low overhead)? Dati kratko obrazloženje.
(a) schedule(static) (b) schedule(static, k) za malo k (c) schedule(dynamic, k) za malo k
(d) a i b (e) b i c

```c
// Code 1A
int n = 1000; float v[n], a[n][n], b[n];
#pragma omp parallel for private(j)
for (i=0; i < n; i++) {
  for (j=i; j < n; j++) {
    v[i] += a[i][j]*b[i];
  }
}
```
```c
// Code 1B
int n = 1000; float v[n], a[n][n], b[n];
#pragma omp parallel for private(j)
for (i=0; i < n; i++) {
  for (j=0; j < n; j++) {
    v[i] += a[i][j]*b[i];
  }
}
```

**H14. [2025_sep]** Optimizovati sledeći kod uklanjanjem nepotrebnih barijera:
```c
#pragma omp parallel private(i)
{
  #pragma omp for
  for(i=0;i<n;i++) a[i] += b[i];
  #pragma omp for
  for(i=0;i<n;i++) c[i] += d[i];
  #pragma omp barrier
  #pragma omp for reduction(+:sum)
  for(i=0;i<n;i++) sum += a[i] + c[i];
}
```

**H15. [2023_okt]** Da li se u sledećem OpenMP programu može iskoristiti `nowait` odredba kod
**prve** paralelne for petlje? Obavezno obrazložiti odgovor.
```c
int n = 1000;
float v[n], a[n][n], b[n];
#pragma omp parallel for private(j)
for (i=0; i < n; i++) {
  for (j=0; j < i; j++) {
    v[i] += a[i][j]*b[i];
  }}
#pragma omp parallel for private(j)
for (i=1; i < n; i++) {
  for (j=i; j < n; j++) {
    v[i] = v[i-1] + a[i][j]*b[i];
  }}
```

## H.4 Pitanja sa ponuđenim odgovorima

**H16. [2025_jan]** Šta je od sledećeg korektan način da se broj niti u OpenMP postavi na 4:
- `omp_get_num_threads(4)` na početku main funkcije
- `omp_num_threads(4)` na početku main funkcije
- U shell napisati `setenv OMP_NUM_THREADS 4`
- `omp_max_threads(4)` na početku main funkcije

**H17. [2025_jun]** Šta od sledećeg **nije tačno**:
- OpenMP je API koji omogućava korišćenje multi-thread paralelizma
- Osnovne komponente OpenMP su kompajlerske direktive, runtime biblioteka, i promenljive okruženja
- OpenMP implementacija postoji za Microsoft Windows platformu
- OpenMP je namenjen za sisteme sa distribuiranom memorijom i garantuje efikasno korišćenje memorije
- OpenMP podržava UMA i NUMA arhitekture

**H18. [2025_jun]** Šta od navedenog **nije moguće** u OpenMP:
- zavisnosti po podacima u `#pragma omp for`
- konflikti po podacima u `#pragma omp critical`
- trka podataka u `#pragma omp parallel`
- deadlock u `#pragma omp parallel`

**H19. [2025_jun]** Koja od navedenih OpenMP direktiva je pogrešno iskorišćena:
- `#pragma omp for nowait`
- `#pragma omp parallel private(threadID)`
- `#pragma omp atomic(threadID)`
- `#pragma omp parallel for if (val)`

**H20. [2025_sep]** Šta znači OpenMP:
Open Multi-Processing / Open Memory Parallelization / Open Message Passing / Open
Multiprocessing Protocol

**H21. [2025_sep]** Koja OpenMP direktiva se koristi da se obezbedi da samo jedna nit izvrši
neki blok naredbi: `#pragma omp master` / `#pragma omp barrier` / `#pragma omp single` /
`#pragma omp critical`

**H22. [2023_jun2 / 2024_okt]** Koje odredbe se mogu koristiti uz direktivu `parallel` u OpenMP?

## H.5 Kratke paralelizacije (spadaju u teorijski deo)

**H23. [2024_jun]** Pomoću OpenMP paralelizovati sledeći program **bez korišćenja reduction
direktive**:
```c
main() {
  int i, k = 0;
  for (i = 1; i <= 100000; i++) { k = k + i; }
  printf("%d\n", k);
}
```

**H24. [2025_apr / 2025_okt2]** Pomoću OpenMP paralelizovati sledeći kod **bez korišćenja
reduction odredbe i pomoću vektora**:
```c
C[0] = 1;
for (i=1; i<N; i++) {
  C[i] = C[i-1];
  for (j=0; j<N; j++) { C[i] *= A[i,j] + B[i,j]; }
}
```

**H25. [2023_jun2 / 2023_dec]** Ubaciti potrebne direktive da bi se paralelizovala sledeća petlja.
Broj niti treba da bude jednak broju procesora u sistemu.
```c
int i;
double A[N] = {...}, B[N] = {...}, C[N], D[N];
const double c = ...;
const double x = ...;
double y;
for (i = 0; i < N; i++)
{
  y = sqrt(A[i]);
  D[i] = y + A[i] / (x * x);
}
```

**H26. [2023_sep]** Paralelizovati petlju po indeksnoj promenljivoj i pomoću OpenMP:
```c
for (i = 0; i < 100 * n; i++) {
  for (j = 0; j < n; j++) { a[i] += b[i][j]; }
  c = foo(c);
}
```

**H27. [2025_okt2]** Korišćenjem OpenMP, paralelizovati izvršenje sledećeg koda:
```c
x = 0;
for (i=0; i<=N; i++)
{ a[i] = x;
  x = x + 2; }
```

**H28. [2024_jan]** Razmotriti i obrazložiti da li je korišćenjem OpenMP direktiva moguće
paralelizovati petlje kojima se traži maksimalna vrednost elemenata kvadratne matrice reda M.

---

# I. GPU / CUDA — konceptualna pitanja

**I1. [2025_okt2 / 2026_apr]**
a) Ako je veličina bloka kod GPU 24x24 niti, koliko warp-ova ima u bloku?
b) Ako SM (streaming multiprocesor) može izvršavati do 1536 niti, koliko ovih blokova se
može izvršavati jednovremeno na SM?
c) Šta bi bilo efikasnije sa stanovišta iskorišćenosti SM, blokovi veličine 8x8 niti,
15x16 (odn. 16x16) niti, ili 24x24 niti, ako se maksimalno 8 blokova može izvršavati u
jednom trenutku?

**I2. [2022_dec]**
a) Zašto se u CUDA kernelima izračunavanja dele dva puta, jednom na nivou rešetke, drugi put
na nivou bloka?
b) Zašto niti iz različitih blokova ne mogu da koriste istu deljivu memoriju?
c) Pretpostaviti da je neki CUDA kernel pokrenut sa 1000 blokova niti, svaki sa 256 niti.
Ako je neka promenljiva deklarisana kao lokalna promenljiva u kernel funkciji, koliko
verzija te promenljive će biti kreirano za vreme izvršavanja kernela?
d) Potrebno je napisati CUDA kernel za sabiranje dva vektora. Ako želimo da svaka pojedinačna
nit računa jedan element rezultujućeg vektora, koji bi izraz bio najpogodniji za mapiranje
indeksa niti i bloka na indeks rezultujućeg vektora:
  i. `i = threadIdx.x + threadIdx.y;`
  ii. `i = blockIdx.x + threadIdx.x;`
  iii. `i = blockIdx.x * blockDim.x + threadIdx.x;`
  iv. `i = threadIdx.x * blockDim.x + blockIdx.x;`
  v. `i = blockIdx.x * threadIdx.x;`
e) Za prethodno navedeni kernel, neka je dužina vektora 5000, neka svaka nit računa samo
jedan element rezultujućeg vektora, i neka je veličina bloka 512 niti. Koliko će ukupno
niti biti u gridu?

**I3. [2023_okt]**
i) Zaokružiti šta sve čini overhead kada se izvršava program na GPU:
a. keš promašaji na host procesoru
b. prenos podataka od hosta ka GPU
c. prenos podataka od GPU ka hostu
d. divergencija niti

ii) Da li su tačne (T) ili netačne (N) sledeće tvrdnje:
- Različite niti u bloku mogu komunicirati preko deljive memorije
- Niti u različitim blokovima mogu komunicirati preko deljive memorije

---

# Dodatak: zadaci koji su izostavljeni (samo popis)

**MPI programerski zadaci:** 2022_dec z.2/z.3, 2023_kol1 z.1, 2023_kol1-popravni z.1/z.2, 2023_jun2 z.1,
2023_sep z.1, 2023_okt z.1, 2023_dec z.1, 2024_jan z.1, 2024_kol1 z.1, 2024_jun z.1, 2024_okt z.1,
2025_jan z.6, 2025_jun z.1, 2025_sep z.1, 2025_okt z.7, 2025_apr z.6, 2025_okt2 z.7, 2026_jan z.6,
2026_apr z.7

**CUDA programerski zadaci:** 2022_dec z.1f, 2023_kol1 z.2, 2023_jun2 z.2, 2023_okt z.2, 2023_dec z.2,
2024_jan z.2, 2024_jun z.2, 2024_okt z.2, 2025_jan z.7, 2025_jun z.2, 2025_sep z.2, 2025_okt z.8,
2025_apr z.7, 2025_okt2 z.8, 2026_jan z.7, 2026_apr z.8

**Scoreboard / Tomasulo:** 2023_jan z.1, 2023_kol1 z.2 (teorija), 2023_jun z.1, 2023_jun2 z.1
*(nema ih ni na jednom blanketu od 2024. naovamo)*

**Veliki "petlja + zavisnosti + OpenMP + testiranje" zadaci** *(struktura je uvek ista:
a) da li je moguća paralelizacija bez modifikacija, b) koje su zavisnosti, c) transformisati,
d) testirati sekvencijalno i paralelno)*:
2022_dec z.4, 2023_jun2 z.3, 2023_okt z.3, 2024_jun z.3, 2024_okt z.3, 2025_jan z.8, 2025_jun z.3,
2025_sep z.3, 2025_okt z.9, 2025_apr z.8 (Flojd-Voršal), 2025_okt2 z.9, 2026_jan z.8, 2026_apr z.9

---

# Statistika pojavljivanja

| Tema | Br. pojavljivanja | Komentar |
|---|---|---|
| **Amdahl / ubrzanje / efikasnost** | **14** | praktično na svakom roku, često 1. zadatak |
| **Write-once / Firefly (stanja + tabele)** | **11** | drugi po učestalosti |
| **Zavisnosti: vektori/pravci, vektorizacija, transformacije** | **10** | |
| **Sprežne mreže (hiperkub, ESC, statičke, izbor mreže)** | **9** | |
| **GCD test** | **6** | uvek ista formulacija, menja se samo kod |
| **Direktorijumske šeme (proračun bitova)** | **4** | |
| **Bernštajnovi uslovi** | **4** | |
| **SP vs VLIW** | **3** | uvek identično pitanje |
| **Memorijski banci / vektorski korak** | **3** | uvek identična formulacija |
| **snoopy vs direktorijumske (konceptualno)** | **3** | |
| **ZIV/SIV/MIV** | **3** | |
| **GPU/CUDA konceptualno** | **3** | |
| **SIMD maskiranje** | **2** | |
| **Arbitraža na magistrali** | **1** | |
| **MIMD čvrsto/slabo spregnuti** | **1** | |
| **OpenMP kratka pitanja (ukupno)** | **28** | razbijeno na mnogo podtipova |
| **Scoreboard / Tomasulo** | 4 | samo 2023, izostavljeno |

## Zadaci koji se doslovno ponavljaju (vredi znati napamet)

1. **Max element na 4-procesorskom sistemu** (X ciklusa send/receive, Y poređenje) —
   2024_okt, 2023_okt, 2025_okt2
2. **32 PE hiperkub, iz 21 u {9, 13, 25, 29}** — 2023_jan, 2023_dec, 2024_jun — *identičan tekst*
3. **Tabela promašaja 4 procesora, x i y u istom bloku** — 2024_okt, 2026_apr — *identična*
4. **Write-once tabela P_Read/P_Write/Read_Blk/Write_Inv** — 2025_jan, 2025_jun
5. **Write-once/Firefly: stanja + komande + promašaj pri upisu** — 2023_jun2, 2024_okt, 2026_apr
6. **3D gnezdo `A[i][j][k] = A[2i-4][j-3][k-4] + …`** — 2023_kol1, 2023_kol1-popravni, 2025_okt
7. **`A(2i+1,5,j) = A(2i-2,N,k) + B(i,j+i,k)`** (ZIV/SIV/MIV) — 2023_jun2, 2024_jun
8. **GCD `A(2p+2q-r+3s) = A(p-6q+5r+3)*B(r+s-4q)`** — 2023_kol1-popravni, 2023_jun, 2025_apr (proširen)
9. **Latentnost / vektorski korak / banke** — 2023_kol1, 2023_kol1-popravni, 2024_kol1, 2025_okt2
10. **SP vs VLIW zaokruživanje** — 2023_kol1, 2023_kol1-popravni, 2024_kol1
11. **`a++; while(a<2);` dve niti** — 2023_sep, 2024_jun
12. **`critical` + `barrier` deadlock** — 2023_jan, 2024_okt
13. **false sharing x, y, z u susednim lokacijama** — 2023_sep, 2025_okt2
14. **`A(i) = A(N-i+1) + C` vektorizacija** — 2025_sep, 2026_apr
15. **40% / 6 delova / 4-jezgarni procesor** — 2024_kol1, 2025_sep, 2026_apr
16. **`if(i%16<8)` ubrzanje na 4 jezgra** — 2023_jan, 2025_apr
17. **`y=sqrt(A[i]); D[i]=y+A[i]/(x*x)`** ubacivanje direktiva — 2023_jun2, 2023_dec
