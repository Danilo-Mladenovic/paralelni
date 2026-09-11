# Keš koherencija — tipovi zadataka i kako se razmišlja

# 0. Osnova

Više keševa može imati kopiju istog bloka. Čim jedan procesor upiše, kopije se razilaze.
Dva pristupa rešenju:

| | Kako radi | Kad se koristi |
|---|---|---|
| **Snoopy** (Write-once, Firefly) | **emisija** komandi preko zajedničke magistrale; svaki keš „njuška" sav saobraćaj | mali/srednji sistemi sa magistralom |
| **Direktorijumska šema** | **evidencija** ko ima koju kopiju; komande se šalju **ciljano** | veliki sistemi, mreže bez broadcast-a |

**Najvažnija razlika u jednoj rečenici:** snoopy emituje svima, direktorijum šalje samo onima
za koje zna da imaju kopiju.

**Zašto je to bitno:** snoopy zahteva **broadcast medijum**. Na slabo spregnutom sistemu
(poruke, bez deljive magistrale) ili sa mnogo procesora — snoopy **nije primenljiv**, mora
direktorijum.

---

# 1. Write-once — kako pamtiš stanja

Ne uči ih kao spisak, nego kao **odgovore na dva pitanja**:

|  | **Konzistentna sa GM** | **NIJE konzistentna sa GM** |
|---|---|---|
| **Može biti više kopija** | **Valid** | — (nemoguće) |
| **Jedina kopija** | **Reserved** | **Dirty** |

Plus **Invalid** = nema važeće kopije.

**Zašto baš tako:** protokol radi **write-through pri PRVOM upisu** (zato Reserved: izmenjeno,
ali GM ažurirana) i **write-back za dalje izmene** (zato Dirty). Ime protokola dolazi odatle —
„write once" kroz GM, pa dalje lokalno.

## Prelazi — izvedi ih, ne pamti

**Pitaj se dva pitanja: (1) pogodak ili promašaj? (2) čitanje ili upis?**

| Situacija | Komanda | Novo stanje | Zašto |
|---|---|---|---|
| Promašaj **čitanja** | `Read_Blk` | **Valid** | pribaviš kopiju, nisi je menjao |
| Promašaj **upisa** | `Read_Inv` | **Dirty** | pribaviš + izmeniš + isteraš ostale; GM **nije** ažurirana |
| Pogodak upisa na **Valid** | `Write_Inv` + ažurira GM | **Reserved** | prvi upis → write-through |
| Pogodak upisa na **Reserved** | **ništa** | **Dirty** | već si jedini, nema koga da obaveštavaš |
| Pogodak upisa na **Dirty** | ništa | Dirty | — |
| Pogodak čitanja (bilo koje stanje) | ništa | nepromenjeno | — |

**Komande koje stižu od DRUGOG procesora (snoop):**

| Stižeš u stanju | Stigne `Read_Blk` | Stigne `Write_Inv`/`Read_Inv` |
|---|---|---|
| Valid | → Valid, ništa | → **Invalid** |
| Reserved | → **Valid**, ništa (GM je ažurna) | → **Invalid** |
| **Dirty** | → **Valid**, **dostavljaš blok + ažuriraš GM** | → **Invalid** (prvo dostaviš/ažuriraš GM) |

**Zamena bloka:** write-back **samo iz Dirty**. Iz Valid i Reserved — ništa, jer je GM ažurna.

---

# 2. Firefly — u čemu je razlika

**Jedna razlika iz koje sve sledi: Firefly AŽURIRA kopije umesto da ih invalidira**
(`Write_Update` umesto `Write_Inv`).

| Stanje | Značenje |
|---|---|
| **Valid-exclusive** | jedina kopija, konzistentna |
| **Shared** | više kopija, sve konzistentne |
| **Dirty** | jedina kopija, nije konzistentna |

**Nema stanja Invalid** — jer se kopije nikad ne invalidiraju, samo ažuriraju.

**Promašaj pri upisu — dva slučaja:**
- nema drugih kopija → blok iz GM → **Dirty**
- ima drugih kopija → `Write_Update` ažurira sve → **sve postaju Shared**

**Poređenje koje se pita:** Write-once završava u **Dirty** sa udaljenim kopijama **Invalid**;
Firefly završava u **Shared** sa svima **ažuriranim**.

**Kad je koji bolji:** ako se deljeni podatak **često čita, retko piše** → Firefly (update
sprečava da svi čitaoci moraju ponovo da pribavljaju blok). Ako se posle upisa retko čita →
Write-once (invalidacija je jeftinija od slanja podataka svima).

---

# 3. TIP: popuni tabelu stanja

**Tok razmišljanja — za svaki red, redom:**

1. **Ko izvršava akciju?** Lokalna komanda (`P_Read`, `P_Write`) ili komanda koja **stiže od
   drugog** (`Read_Blk`, `Write_Inv` bez prefiksa)?
2. **Pogodak ili promašaj?** Ako je stanje Invalid ili prazno → promašaj.
3. **Primeni prelaz** iz tabela gore.
4. **Ažuriraj i OSTALE kešove** — ako si emitovao `Write_Inv`/`Read_Inv`, svi ostali → Invalid.
5. **Ažuriraj GM** — samo ako komanda to traži.

## Tri zamke koje se najviše naplaćuju

**Zamka 1 — GM „kasni jedan korak".** GM se ažurira **samo** kad: (a) pogodak upisa na Valid
(write-through), ili (b) Dirty kopija mora da preda blok nekom drugom. Promašaj pri upisu vodi
u Dirty **bez** ažuriranja GM — pa GM ostaje stara.

**Zamka 2 — x i y u ISTOM bloku (lažno deljenje).** Ako zadatak kaže da su dve promenljive u
istom keš bloku, onda **operacija nad y menja status x**. Pristup promenljivoj koju uopšte
nisi dirao može biti **promašaj** jer je blok invalidiran zbog druge promenljive. Ako su u
**različitim** blokovima — linije su potpuno nezavisne.

**Zamka 3 — zamena bloka kad keš drži samo jedan blok.** Pre nego što upišeš novo stanje,
pitaj se: **da li je stari blok bio Dirty?** Ako jeste → write-back. Ako je bio **Reserved** →
**nema** write-back-a (GM je već ažurna) — ovo se najčešće promaši.

---

# 4. TIP: proračun bitova stanja

**Uvek prvo izračunaj dve stvari:**
```
broj_mem_blokova = kapacitet_GM / veličina_bloka
broj_keš_blokova = kapacitet_keša / veličina_bloka        (po JEDNOM procesoru)
m = log₂(broj_procesora)
```

Zatim po šemi:

| Šema | Po memorijskom bloku (u GM) | Po keš bloku |
|---|---|---|
| **Write-once** | — (nema direktorijuma) | **2** (4 stanja) |
| **Potpuno preslikan** | **N + 1** (bit po procesoru + dirty) | 2 |
| **Ograničen, max k kopija** | **k·m + 1** (k pokazivača + dirty) | 2 |
| **Ulančan** | **1 + m** (valid + pokazivač na prvog) | **m** (pokazivač na sledećeg) |

```
UKUPNO = (po mem. bloku) × broj_mem_blokova
       + (po keš bloku)  × broj_keš_blokova × broj_procesora
```

**Šta se zapravo ispituje — rast sa brojem procesora:**
- potpuno preslikan raste **linearno** sa N (N+1 bita)
- ograničen i ulančan rastu sa **log₂N**

Zato za velike sisteme potpuno preslikan postaje neupotrebljiv — u tipičnom zadatku ispadne
da direktorijum zauzima **preko 50% same glavne memorije**. To je poenta, ne sam broj.

**Ako zadatak traži samo „veličinu direktorijuma u GM"** → računaš samo prvi red. Ako traži
„ukupno bitova stanja" → i keš stranu.

## Podtip: „koju šemu izabrati"

**Tok razmišljanja — dva kriterijuma koja se sukobljavaju:**
1. **Utrošak bitova** — Write-once je uvek nesrazmerno jeftiniji (nema direktorijuma)
2. **Primenljivost** — Write-once traži **magistralu sa broadcast-om**

Odgovor mora pomiriti oba: sa mnogo procesora Write-once je jeftiniji ali **arhitektonski
neupotrebljiv** (magistrala se zasiti), a potpuno preslikan je upotrebljiv ali **memorijski
neprihvatljiv** → praktičan izbor je **ograničen ili ulančan adresar**.

---

# 5. Kako rade direktorijumske šeme (za tabele)

## Ulančan adresar
Prvi čitalac dobija kopiju + **CT** (chain termination) pokazivač; GM pokazuje na njega.
Svaki sledeći čitalac dobija pokazivač na **prethodnog**, a GM se ažurira da pokazuje na
**najnovijeg**. Kod upisa invalidacija ide **kroz ceo lanac**; upis se ne dozvoljava dok ne
stigne potvrda od onog sa CT pokazivačem.

## Ograničen adresar — ovde je cela poenta u ograničenju

Bitovi u kešu: **V** (valid) i **W** (write). U GM: do k pokazivača + **D** (dirty).

**Tok razmišljanja za tabelu:**
- **čitanje, ima mesta** → dodaj pokazivač, V=1, W=0
- **upis** → direktorijum **invalidira sve ostale**, ostaje samo pisac, **D=1**, W=1
- **čitanje dok je D=1** → vlasnik radi **write-back**, D→0, W→0, oba postaju čitaoci
- **čitanje kad je direktorijum PUN (k od k)** → **postojeća kopija se mora izbaciti**

**To poslednje je poenta zadatka:** kod ograničenog adresara **savršeno validna kopija biva
invalidirana iako niko nije pisao** — samo zato što je stigao (k+1)-vi čitalac. To izaziva
nepotrebne promašaje; potpuno preslikan to nema, ali plaća linearnim rastom.

---

# 6. Kontrolna lista

- Je li komanda **lokalna** ili stiže **od drugog** procesora? (menja ceo prelaz)
- Kod promašaja pri upisu — završavaš u **Dirty**, GM se **NE** ažurira
- Kod pogotka upisa na Valid — GM se **ažurira** (write-through), stanje **Reserved**
- Kad Dirty kopija mora da preda blok — **prvo ažurira GM**, pa prelazi u Valid
- Zamena bloka: write-back **samo iz Dirty**, ne iz Reserved
- Jesu li promenljive u **istom** bloku? → operacija nad jednom utiče na drugu
- Kod proračuna — jesi li razlikovao „direktorijum u GM" od „ukupno bitova stanja"?
- Kod ograničenog adresara — jesi li primetio kad direktorijum **prekipi**?
