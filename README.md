# IKP_PubSub_Engine

**Publish-Subscribe distribuirani sistem za industrijske komunikacijske protokole (C++17)**

Centralizovani PubSub engine sa paralelnom dostavom poruka, podrškom za više topika, automatskom validacijom i HTTP monitoringom u realnom vremenu.

---

## 📋 Sadržaj

- [Potrebni Alati](#potrebni-alati)
- [Setup i Kompajliranje](#setup-i-kompajliranje)
- [Pokretanje Servisa](#pokretanje-servisa)
- [HTTP Monitoring Dashboard](#http-monitoring-dashboard)
- [Struktura Projekta](#struktura-projekta)
- [Opis Klasa](#opis-klasa)

---

## 🔧 Potrebni Alati

### Obavezni

| Alat | Verzija | Opis |
|------|---------|------|
| **MinGW-w64** | 8.0+ | GCC kompajler sa Windows Sockets (Winsock2) podrškom |
| **C++** | C++17 | Najmanje verzija 17 za `std::thread`, `std::atomic` |
| **Winsock2** | 2.2+ | Windows sockets biblioteka (obično dolazi sa MinGW) |

### Opcionalni

- **PowerShell 5.0+** - Za pokretanje `compile.ps1` skripte
- **Git** - Za verzionisanje koda
- **VS Code** / **Visual Studio** - Za razvoj i debug

### Proverite instalaciju

```batch
where g++
g++ --version
```

**Očekivani rezultat:**
```
g++ (Rev8, Built by MSYS2 project) 15.2.0
```

---

## 🚀 Setup i Kompajliranje

### 1. Provera Setup-a

```batch
.\check_setup.bat
```

**Očekivani ispis:**
```
[OK] g++ found
[OK] C++17 support available
[OK] src/ directory found
[OK] src/core/ directory found
[OK] src/utils/ directory found
```

### 2. Kompajliranje Projekta

```batch
.\compile.bat
```

**Očekivani rezultat:**
```
========================================
Build complete!
========================================

Run services with:
  Engine:     .\pubsub.exe --engine
  Publisher:  .\pubsub.exe --publisher
  Subscriber: .\pubsub.exe --subscriber --topic "Analog/MER/220"
```

---

## 🎯 Pokretanje Servisa

Otvorite **tri zasebna terminala** i pokrenite servise **ovim redom**:

### Terminal 1 — Engine

```batch
.\pubsub.exe --engine
```

**Očekivani ispis:**
```
[PubSubEngine] Engine started on port 5000
[PubSubEngine] Subscriber validation thread started
[HttpServer] Dashboard dostupan na: http://localhost:8080
```

> ⚠️ Engine mora biti pokrenut **prvi**, prije publisher-a i subscriber-a.

---

### Terminal 2 — Publisher

```batch
.\pubsub.exe --publisher
```

**Sa opcionalnim parametrima:**

| Parametar | Opis | Default |
|-----------|------|---------|
| `--port <broj>` | Port za publisher | auto-assign od 4100 |
| `--engine-host <host>` | Engine host adresa | localhost |
| `--engine-port <broj>` | Engine port | 5000 |

**Primeri:**
```batch
.\pubsub.exe --publisher
.\pubsub.exe --publisher --port 4101
.\pubsub.exe --publisher --port 4101 --engine-host 192.168.1.100
```

**Što radi:**
- Šalje poruke svake **2 sekunde**, rotacijom po topicima
- Podrazumevani topici: `Analog/MER/220`, `Status/SWG/1`, `Status/CRB/1`
- Analogni signali simuliraju realni napon sa random walk algoritmom i 10% šansom za spike

**Očekivani ispis:**
```
[localhost:4101] Povezan na engine, sluza na portu 4101
[localhost:4101] ANALOG: Vrednost=219.43 | Topic=Analog/MER/220
[localhost:4101] STATUS: Vrednost=SWG_OPEN | Topic=Status/SWG/1
```

---

### Terminal 3 — Subscriber

```batch
.\pubsub.exe --subscriber --topic "Analog/MER/220"
```

**Sa opcionalnim parametrima:**

| Parametar | Opis | Default |
|-----------|------|---------|
| `--topic <string>` | Topic za pretplatu (može više puta) | — (obavezno) |
| `--port <broj>` | Port za subscriber | auto-assign od 4200 |
| `--engine-host <host>` | Engine host adresa | localhost |
| `--engine-port <broj>` | Engine port | 5000 |

**Primeri:**
```batch
REM Jedan topic
.\pubsub.exe --subscriber --topic "Analog/MER/220"

REM Vise topika
.\pubsub.exe --subscriber --topic "Analog/MER/220" --topic "Status/CRB/1"

REM Sa specificiranim portom
.\pubsub.exe --subscriber --topic "Status/SWG/1" --port 4201
```

**Dostupni topici:**

```
Analog/MER/220    — Merenje napona (float vrijednost)
Analog/MER/400    — Merenje napona 400kV (float vrijednost)
Status/SWG/1      — Switchgear status (SWG_OPEN / SWG_CLOSED)
Status/CRB/1      — Circuit Breaker status (CRB_OPEN / CRB_CLOSED)
```

**Očekivani ispis:**
```
[localhost:4201] Pretplaćen na: Analog/MER/220

--------------------------------------
PUBLISHER: localhost:4101 | PORUKA #1 | 14:23:05
Topic: Analog/MER/220
Tip: ANALOG
Vrednost: 219.43
```

---

### Kompletan primer sa 5 terminala

```batch
# Terminal 1
.\pubsub.exe --engine

# Terminal 2
.\pubsub.exe --publisher --port 4101

# Terminal 3
.\pubsub.exe --publisher --port 4102

# Terminal 4
.\pubsub.exe --subscriber --topic "Analog/MER/220" --port 4201

# Terminal 5
.\pubsub.exe --subscriber --topic "Status/CRB/1" --port 4202
```

---

### Izlazak iz servisa

U bilo kom terminalu:
```
exit
```

---

## 🌐 HTTP Monitoring Dashboard

Engine automatski pokreće HTTP server na portu **8080**.

Nakon pokretanja engine-a, otvorite browser i idite na:

```
http://localhost:8080
```

### Šta prikazuje dashboard:

| Sekcija | Opis |
|---------|------|
| **Kartice (gore)** | Uptime, ukupno poruka, broj aktivnih topika, broj pretplatnika |
| **Linijski grafikon** | Analogne vrijednosti u realnom vremenu (poslednjih 20 mjerenja) |
| **Digitalni signali** | Trenutni status SWG/CRB topika (OPEN/CLOSED) |
| **Tabela topika** | Pregled svih aktivnih topika sa brojem poruka i zadnjom vrijednošću |
| **Log poruka** | Poslednjih 20 poruka hronološki |
| **Alarm panel** | Prikazuje se automatski ako analogna vrijednost odstupa više od 10 od nominalne |

### API endpoint:

```
GET http://localhost:8080/api/stats
```

Vraća JSON sa svim statistikama engine-a (topici, poruke, uptime, pretplatnici).

---

## 📁 Struktura Projekta

```
IKP_PubSub_Engine/
├── README.md
├── compile.bat
├── compile.ps1
├── check_setup.bat
├── pubsub.exe
│
└── src/
    ├── Message.h                       # Struktura poruke
    ├── Network.h/cpp                   # TCP klijent/server, PortPool, ConsoleHandler
    ├── Serialization.h                 # Binarna serijalizacija poruka
    │
    ├── core/
    │   ├── PubSubEngine.h/cpp          # Centralni engine
    │   ├── Publisher.h/cpp             # Izdavač poruka
    │   └── Subscriber.h/cpp            # Primač poruka
    │
    ├── monitor/
    │   └── HttpServer.h/cpp            # HTTP server za monitoring dashboard
    │
    ├── utils/
    │   ├── MessageValidator.h/cpp      # Validacija poruka
    │   ├── MessageFormatter.h/cpp      # Formatiranje za ispis
    │   ├── CommandLineParser.h/cpp     # Parsiranje CLI parametara
    │   └── NetworkUtils.h/cpp          # Pomoćne mrežne funkcije
    │
    ├── DataStructures/
    │   ├── LinkedList.h                # Ulančana lista (za subscriber liste)
    │   ├── CircularBuffer.h            # Kružni bafer (za recent poruke)
    │   └── HashMap.h                   # Hash mapa (O(1) lookup topika)
    │
    └── main.cpp                        # Entry point sa mode selection
```

---

## 📚 Opis Klasa

### 🎯 Core (`src/core/`)

#### PubSubEngine
Centralna komponenta sistema. Upravlja registracijom publisher-a i subscriber-a, rutira poruke prema topicima i vrši paralelnu dostavu (svaki subscriber u posebnom thread-u). Svakih 5 sekundi provjerava dostupnost subscriber-a i uklanja nedostupne. Čuva statistike za HTTP monitoring.

#### Publisher
Konekcija na engine i periodično slanje poruka (svake 2 sekunde). Implementira random walk algoritam za simulaciju realnih analognih mjerenja sa 10% šansom za spike vrijednost van normalnog opsega.

#### Subscriber
Otvara server socket na specifičnoj porti, registruje se za željene topice i prima poruke od engine-a. Validira i ispisuje svaku primljenu poruku.

---

### 🌐 Monitor (`src/monitor/`)

#### HttpServer
Standalone HTTP server koji se pokreće zajedno sa engine-om. Servira HTML dashboard na portu 8080 i JSON API na `/api/stats`. Dashboard se automatski osvježava svake 2 sekunde i prikazuje stanje sistema u realnom vremenu.

---

### 🛠️ Utils (`src/utils/`)

#### MessageValidator
Validacija poruke prije slanja — provjerava tip, topic format i opseg vrijednosti (ANALOG: float, STATUS: OPEN/CLOSED/SWG_OPEN/SWG_CLOSED/CRB_OPEN/CRB_CLOSED).

#### MessageFormatter
Formatiranje poruke u čitljiv string za konzolni ispis.

#### CommandLineParser
Parsiranje CLI argumenata (`--engine`, `--publisher`, `--subscriber`, `--topic`, `--port`, itd.).

#### NetworkUtils
Kodiranje i dekodiranje dužine poruke u 4-byte big-endian formatu.

---

### 📊 DataStructures (`src/DataStructures/`)

| Klasa | Upotreba | Složenost |
|-------|----------|-----------|
| `LinkedList<T>` | Lista subscriber-a po topicu | O(n) pristup |
| `CircularBuffer<T>` | Recent poruke po topicu (50 mjesta) | O(1) push/pop |
| `HashMap<K,V>` | Lookup topika po imenu | O(1) prosječno |

---

## 🔄 Tok podataka

```
Publisher
  └─ connect(engine:5000)
  └─ publish(msg) svake 2s
       │
       ▼
PubSubEngine (port 5000)
  └─ findTopic(msg.topic)
  └─ updateStats()           ──► HttpServer (port 8080) ──► Browser
  └─ za svakog subscribera:
       └─ spawn thread
            └─ deliver(msg)
                 │
                 ▼
           Subscriber (port 420x)
             └─ validate(msg)
             └─ print(msg)
```

---

## 🐛 Troubleshooting

| Problem | Rješenje |
|---------|----------|
| `g++ not found` | Instalirajte MinGW sa https://www.mingw-w64.org/ |
| `Cannot connect to engine` | Pokrenite engine prvi, provjerite port 5000 |
| Subscriber ne prima poruke | Topic mora biti identičan (case-sensitive, npr. `Analog/MER/220`) |
| Dashboard ne radi | Provjerite da engine radi i idite na `http://localhost:8080` |
| Port već zauzet | Koristite `--port` parametar da zadate drugi port |

---

