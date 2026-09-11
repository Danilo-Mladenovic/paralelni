# Sprežne mreže — tipovi zadataka i kako se razmišlja

# 0. Osnova

**Adrese su binarne.** Sve u ovoj oblasti se svodi na to koji se bitovi adrese razlikuju
između izvora i odredišta.

Za **N = 2^m** procesnih elemenata:
```
m = log₂N                  bitova u adresi = broj stepena mreže
N/2                        komutacionih elemenata (KE) po stepenu
(N/2)·m                    ukupno KE
```

**Sprežna funkcija Cᵢ** komplementira i-ti bit adrese. Jedan stepen mreže = jedna Cᵢ.
Odatle sledi sve ostalo: **da bi se sa adrese S stiglo na D, treba komplementirati tačno one
bitove u kojima se razlikuju** — a to je `S ⊕ D`.

---

# 1. TIP: rutiranje jedan-na-jedan (zaglavlje)

```
T = S ⊕ D
tᵢ = 0 → KE u stepenu i ide DIREKTNO
tᵢ = 1 → KE u stepenu i ide UKRŠTENO
```

**Zašto:** ako se S i D poklapaju na bitu i, taj bit ne treba dirati → direktno. Ako se
razlikuju, treba primeniti Cᵢ → ukršteno. **Zaglavlje je bukvalno spisak bitova koje treba
promeniti.**

**Isto važi i za statički hiperkub**, samo se tamo ne postavljaju KE nego se ide skok po skok:
menjaj **po jedan bit, s leva na desno**, dok se S ne izjednači sa D.
```
1001 → 0001 → 0101 → 0111 → 0110      (S=1001, D=0110)
```
**Broj skokova = broj jedinica u S ⊕ D.**

---

# 2. TIP: emisija (broadcast) — jedan izvor, više odredišta

## Korak 1 — provera izvodljivosti (NE preskačeš)

Emisija u jednom prolazu je moguća **samo ako**:
- broj odredišta je **stepen dvojke**, 2ʲ
- odredišne adrese se **razlikuju na tačno j pozicija** i **poklapaju na preostalih m−j**

**Kako brzo proveriš:** XOR-uj svako odredište sa prvim i sve to OR-uj. Broj jedinica u
rezultatu mora biti **tačno j**.

Ako uslov padne → **emisija nije moguća**, mora se slati u više talasa ili pojedinačno.

## Korak 2 — zaglavlje
```
R = S ⊕ X    za bilo koje odredište X        (put do tačke grananja)
B = X ⊕ Y    za bilo koja dva odredišta      (tačke grananja)

zaglavlje = {R, B}
```

**Zašto B ne zavisi od S:** `Tₓ ⊕ Tᵧ = (S⊕X) ⊕ (S⊕Y) = X ⊕ Y` — S se poništi. Zato B možeš
računati direktno iz odredišta, brže i sigurnije.

**Šta B znači:** jedinice u B su pozicije na kojima se odredišta razlikuju — tu KE mora
**granati** signal u oba smera umesto da ga prosledi jednim putem.

---

# 3. TIP: ESC mreža (otpornost na otkaz)

**Šta je:** generalizovani kub sa **dodatnim stepenom 0**. Normalno je premošćen; kad se
detektuje greška, aktivira se i tako nastaju **dva disjunktna puta** između svakog para.

**Kako se pišu zaglavlja — mehanički:**
```
Primarni:    0  +  T                              (vodeća 0, T nepromenjeno)
Sekundarni:  1  +  T sa komplementiranim POSLEDNJIM bitom
```

**Za emisiju:** izračunaj `{R, B}` normalno, pa primeni isto pravilo **na R**:
```
Primarni:    R = 0 + R
Sekundarni:  R = 1 + R(poslednji bit komplementiran)
B            = 0 + B                              (B dobija samo vodeću nulu)
```

**Zašto B ostaje netaknuto:** B opisuje **grananje unutar** mreže, a dodati stepen nije tačka
grananja — samo alternativni ulaz.

---

# 4. TIP: sprežne funkcije — napiši ih

Za adresu `b(m−1) … b₁ b₀`:

| Funkcija | Šta radi |
|---|---|
| **Mešanje** (shuffle) | ciklična rotacija **ulevo** za 1: `b(m−2)…b₁b₀b(m−1)` |
| **Zamena** (exchange) | komplementira **najniži** bit: `b(m−1)…b₁~b₀` |
| **Kub** Cᵢ | komplementira **i-ti** bit |
| **Rešetka** (mesh) | 4 funkcije mod N: `(x±1) mod N` i `(x±r) mod N`, gde je **r = √N** |

Mešanje-zamena je **par** funkcija (uvek se navode obe). Kub je **skup od m** funkcija.

---

# 5. TIP: statičke topologije — stepen čvora i dijametar

**Stepen čvora** = broj direktnih suseda. **Dijametar** = najduži od najkraćih puteva.

| Topologija | Stepen | Dijametar |
|---|---|---|
| Linearna | 2 | n−1 |
| Prsten | 2 | n/2 |
| Zvezda | n−1 / 1 | 2 |
| Rešetka k×k (N=k²) | 4 | 2(k−1) |
| Potpuno povezana | n−1 | 1 |
| **Hiperkub** (N=2^m) | **log₂N** | **log₂N** |

**Pitanje koje se javlja: „koja mreža ima stepen čvora koji raste logaritamski?"**
→ **hiperkub**. Za N=2^m svaki čvor je povezan sa **m** čvorova — onima čija se adresa
razlikuje u **tačno jednom bitu**. To je i razlog zašto je dijametar isti: najgori slučaj je
kad se razlikuju svi bitovi, dakle m skokova.

---

# 6. TIP: „koju mrežu izabrati" (npr. za 625 procesora)

**Tok razmišljanja — uvek isti tri kriterijuma:**

1. **Cena** (broj veza/KE)
2. **Propusnost** (koliko prenosa istovremeno)
3. **Rutiranje** (dijametar — koliko skokova u najgorem slučaju)

| Mreža | Cena | Propusnost | Rutiranje |
|---|---|---|---|
| Magistrala | najniža | **najgora** — samo 1 prenos u trenutku | trivijalno |
| Potpuno povezana | **neizvodljiva** (n(n−1)/2 veza) | najbolja | dijametar 1 |
| Rešetka | prihvatljiva | dobra | dijametar 2(√N−1) |

**Odgovor je uvek kompromis**, i mora se obrazložiti brojevima: za 625 procesora potpuno
povezana traži **194 700** veza (neizvodljivo), magistrala se zaguši, a **rešetka 25×25**
(jer 25²=625) daje razumnu cenu uz dijametar od 48 skokova.

## Podtip: koliko traje zadati obrazac komunikacije

*Npr. tri procesora šalju istovremeno: P1→P2, P2→P3, P3→P1.*

**Tok razmišljanja — za svaku mrežu pitaj dva pitanja:**
1. **Koliko poruka može ići istovremeno?** (magistrala: **jedna**, jer je jedan deljivi medijum)
2. **Da li dve poruke dele istu vezu?** Ako da → serijalizuju se. Ako svaka ide svojom vezom
   → sve istovremeno.
3. **Koliko skokova treba svakoj poruci?** (nema direktne veze → prolazi kroz posrednika)

*Ishod za gornji primer:* magistrala **3** (serijalizacija), linearna **3** (P3→P1 nema
direktnu vezu, treba 2 skoka), prsten **1** (sve tri poruke su jedan skok preko **tri
različite veze**).

---

# 7. TIP: SIMD maskiranje

Tri načina, i **cela poenta zadatka je razlika u tome šta koji može**:

| Način | Kako radi | Šta može |
|---|---|---|
| **Direktno** | **1 bit po PE** (maska dužine N), određuje kompajler | **proizvoljan podskup** ✔ |
| **Adresom PE** | **m-bitna** maska sa vrednostima **0 / 1 / X** | samo **podkubove** ✘ |
| **Podacima** | svaki PE **sam** testira uslov nad svojim podacima i postavlja flag | grananje zavisno od **lokalnih podataka** |

## Kako se rešava „dozvoli rad PE-ovima {…}"

**Direktna maska** — trivijalno: napiši N bitova, 1 na pozicijama aktivnih PE. **Uvek radi.**

**Adresna maska** — tok razmišljanja:
1. Napiši adrese aktivnih PE binarno
2. Pitaj se: **postoji li kombinacija 0/1/X koja pokriva tačno te adrese i nijednu drugu?**
   - `X` na poziciji znači „oba bita" → jedna maska pokriva **2^(broj X-ova)** adresa
   - zato jedna maska uvek pokriva **stepen dvojke** adresa, i to onih koje čine **podkub**
3. Ako skup nije podkub → **jedna maska nije dovoljna**, razloži ga na više podkubova

*Primer:* {0,1,3,9,12} → 5 adresa, nije stepen dvojke → sigurno treba više maski.
`000X` pokriva {0,1}, pa `0011`, `1001`, `1100` → **4 maske**.

**Maskiranje podacima** je jedino koje realizuje `where (uslov) … elsewhere …`:
emituj granu A (rade PE sa flag=1) → **komplementiraj flag** → emituj granu B.

---

# 8. TIP: arbitraža na magistrali

**Centralizovana sa lančanim zahvatanjem — kako radi:**
1. Svi šalju zahtev preko zajedničke linije **BUS REQUEST** (žičano ILI — arbitar vidi *da
   ima* zahteva, ne *ko* ga je poslao)
2. Arbitar šalje **BUS GRANT** koji putuje kroz **lanac** gospodara
3. Gospodar koji **jeste** tražio **ne prosleđuje** GRANT dalje i aktivira **BUS BUSY**
4. Po završetku oslobađa BUS BUSY → novi ciklus

**Šta iz ovoga sledi (to se i pita):**
- **Prioritet = fizička pozicija** u lancu; bliži arbitru presreće signal prvi
- **Prednost:** jednostavnost, malo linija
- **Nedostatak:** **izgladnjivanje** najdaljeg gospodara

**Distribuirana arbitraža:** nema centralnog arbitra; svaki gospodar ima **arbitracioni broj**,
svi ih šalju na deljive linije, formira se zbirni broj, **pobeđuje najviši**.

---

# 9. Kontrolna lista

- Kod emisije — jesi li **prvo** proverio da je broj odredišta stepen dvojke i da se
  razlikuju na tačno j pozicija?
- B računaš kao `X ⊕ Y` (iz odredišta) — brže i bez greške nego preko Tₓ ⊕ Tᵧ
- Kod ESC-a — komplementiraš **poslednji** bit, i to samo kod **sekundarnog** puta; B dobija
  samo vodeću nulu
- Kod hiperkuba — broj skokova je **broj jedinica** u S ⊕ D
- Kod adresne maske — je li skup uopšte **stepen dvojke** po veličini? Ako nije, sigurno treba
  više maski
- Kod izbora mreže — jesi li naveo **brojeve** (veze, dijametar), a ne samo „bolje/gore"?
- Kod obrasca komunikacije — dele li dve poruke **istu vezu**?
