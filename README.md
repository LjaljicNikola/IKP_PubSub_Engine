# IKP_PubSub_Engine

**Publish-Subscribe distribuirani sistem za industrijske komunikacijske protokole (C++17)**

Centralizovani PubSub engine sa paralelnom dostavom poruka, podrškom za više topika i automatskom validacijom.

---

## 📋 Sadržaj

- [Potrebni Alati](#potrebni-alati)
- [Setup i Kompajliranje](#setup-i-kompajliranje)
- [Pokretanje Servisa](#pokretanje-servisa)
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

**Windows (cmd.exe ili PowerShell):**
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

Pokrenite skriptu za proveru da li su svi potrebni alati dostupni:

```batch
.\check_setup.bat
```

**Šta se proverava:**
- ✅ g++ kompajler u PATH
- ✅ C++17 podrška
- ✅ Direktorijumi: `src/`, `src/core/`, `src/utils/`

**Očekivani ispis:**
```
[OK] g++ found
[OK] C++17 support available
[OK] src/ directory found
[OK] src/core/ directory found
[OK] src/utils/ directory found
```

### 2. Kompajliranje Projekta

**Opcija A: Batch skripte (cmd.exe)**
```batch
.\compile.bat
```

**Opcija B: PowerShell**
```powershell
.\compile.ps1
```

Obe skripte:
- Čiste prethodne object fajlove (`.o`)
- Kompajliraju svih 9 izvornih fajlova
- Linkuju u `pubsub.exe` (640 KB)
- Ispisuju status greške ako postoje

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

### Modni Razvoj: 3 Terminala

Otvorite **tri zasebna terminala** (PowerShell ili cmd.exe) i pokrenite servise:

#### Terminal 1: PubSub Engine

```batch
.\pubsub.exe --engine
```

**Opcije:**
- Nema dodatnih parametara
- Startuje na **port 5000** (fiksni)
- Čeka konekcije publisher-a i subscriber-a
- Prikazuje logove dostave poruka

**Očekivani ispis:**
```
[PubSubEngine] Engine started on port 5000
[PubSubEngine:VALIDATION] Subscriber health check thread started
[PubSubEngine:DELIVERY] Message published to topic 'Analog/MER/220' -> Subscriber on port 4201 [SUCCESS]
```

---

#### Terminal 2: Publisher

```batch
.\pubsub.exe --publisher --port 4101
```

**Parametri:**

| Parametar | Opis | Primer | Obavezno |
|-----------|------|--------|----------|
| `--publisher` | Pokreni Publisher modu | `--publisher` | ✅ Da |
| `--port <broj>` | Specificiraj port za publisher | `--port 4101` | ❌ Ne (auto-assign ako se izostavi) |
| `--engine-host <host>` | Engine host adresa | `--engine-host localhost` | ❌ Ne (default: localhost) |
| `--engine-port <broj>` | Engine port | `--engine-port 5000` | ❌ Ne (default: 5000) |

**Primeri:**

```batch
REM Publisher na auto-dodeljenoj porti, engine na lokalnoj mašini
.\pubsub.exe --publisher

REM Publisher na porti 4101, koristeci engine localhost:5000
.\pubsub.exe --publisher --port 4101

REM Publisher na porti 4101, engine na drugoj mašini
.\pubsub.exe --publisher --port 4101 --engine-host 192.168.1.100 --engine-port 5000

REM Publisher na specifičnom engine-u
.\pubsub.exe --publisher --port 4102 --engine-host remote-server.local --engine-port 5000
```

**Šta radi:**
- 📤 Šalje **3 vrste poruka** na svakih 10 sekundi:
  - `Analog/MER/220` (merenje napona)
  - `Status/SWG/1` (status switchgear-a)
  - `Status/CRB/1` (status circuit breaker-a)
- 🔄 Rotira tip poruke (ANALOG → SWG_STATUS → CRB_STATUS)
- 📋 Prikazuje svaku poslatu poruku sa timestamp-om

**Očekivani ispis (Publisher na portu 4101):**
```
[localhost:4101] Povezan na engine, sluza na portu 4101
[localhost:4101] Pokrenut thread za objavljivanje
[localhost:4101] ANALOG: Vrednost=3524.5 | Topic=Analog/MER/220
[localhost:4101] STATUS: Vrednost=CLOSED | Topic=Status/CRB/1
[localhost:4101] STATUS: Vrednost=SWG_OPEN | Topic=Status/SWG/1
```

---

#### Terminal 3: Subscriber

```batch
.\pubsub.exe --subscriber --topic "Analog/MER/220" --port 4201
```

**Parametri:**

| Parametar | Opis | Primer | Obavezno |
|-----------|------|--------|----------|
| `--subscriber` | Pokreni Subscriber modu | `--subscriber` | ✅ Da |
| `--topic <string>` | Topic za pretplatu (može se ponavljati) | `--topic "Analog/MER"` | ✅ Da (min. 1) |
| `--port <broj>` | Port za subscriber | `--port 4201` | ❌ Ne (auto-assign ako se izostavi) |
| `--engine-host <host>` | Engine host adresa | `--engine-host localhost` | ❌ Ne (default: localhost) |
| `--engine-port <broj>` | Engine port | `--engine-port 5000` | ❌ Ne (default: 5000) |

**Primeri:**

```batch
REM Subscriber na portu 4201, sluša Analog/MER
.\pubsub.exe --subscriber --topic "Analog/MER/220" --port 4201

REM Subscriber na portu 4202, sluša Status/CRB
.\pubsub.exe --subscriber --topic "Status/CRB/1" --port 4202

REM Subscriber sluša VIŠE topika
.\pubsub.exe --subscriber --topic "Analog/MER/220" --topic "Status/CRB/1" --port 4203

REM Subscriber sa specifičnim engine-om
.\pubsub.exe --subscriber --topic "Analog/MER" --port 4201 --engine-host 192.168.1.50 --engine-port 5001

REM Subscriber sa auto-dodeljenoj porti (starting from 4200)
.\pubsub.exe --subscriber --topic "Status/SWG/1"
```

**Dostupni Topici:**

Postojeći topici koje Publisher šalje:

```
Analog/MER/220    - Merenja napona (vrednost: float -1000...10000)
Status/SWG/1      - Switchgear status (vrednost: OPEN/CLOSED)
Status/CRB/1      - Circuit Breaker status (vrednost: OPEN/CLOSED)
```

**Šta radi:**
- 👂 Sluša na zadatoj porti
- 🎯 Prima SAMO poruke koje match topic-u
- ✅ Validira tip poruke (ANALOG vs STATUS)
- 📊 Prikazuje svaku primljenu poruku sa publisher info-om

**Očekivani ispis (Subscriber na portu 4201, topic="Analog/MER/220"):**
```
[localhost:4201] Pretplaćen na: Analog/MER/220
[localhost:4201] Pokrenut thread za primanje poruka

--------------------------------------
PUBLISHER: localhost:4101 | PORUKA #1 | 03:48:44
Topic: Analog/MER/220
Tip: ANALOG
Vrednost: 3524.5

--------------------------------------
PUBLISHER: localhost:4101 | PORUKA #2 | 03:48:54
Topic: Analog/MER/220
Tip: ANALOG
Vrednost: 4012.3
```

---

### 📝 Kompletan Primer (5 Terminala)

**Terminal 1 (Engine):**
```powershell
PS> .\pubsub.exe --engine
```

**Terminal 2 (Publisher 1):**
```powershell
PS> .\pubsub.exe --publisher --port 4101
```

**Terminal 3 (Publisher 2):**
```powershell
PS> .\pubsub.exe --publisher --port 4102
```

**Terminal 4 (Subscriber - Analog):**
```powershell
PS> .\pubsub.exe --subscriber --topic "Analog/MER/220" --port 4201
```

**Terminal 5 (Subscriber - Status):**
```powershell
PS> .\pubsub.exe --subscriber --topic "Status/CRB/1" --port 4202
```

---

### Izlazak iz Servisa

U bilo kom terminalu, ukucajte:
```
exit
```

Servis će se graciozno isključiti sa:
```
[localhost:XXXX] Shutdown initiated
[localhost:XXXX] Zaustavljen thread
```

---

## 📁 Struktura Projekta

```
IKP_PubSub_Engine/
├── README.md                      # Dokumentacija (ovaj fajl)
├── compile.bat                    # Batch skripte za Windows
├── compile.ps1                    # PowerShell skripte za build
├── check_setup.bat                # Provera okruženja
├── Makefile                       # Build fajl (alternativa)
├── pubsub.exe                     # Kompajlirani binarni
│
└── src/                           # Izvorni kod
    │
    ├── Message.h                  # Struktura poruke sa tipom, topikom, vrednosti
    ├── Network.h/cpp              # TCP klijent/server, PortPool, ConsoleHandler
    ├── Serialization.h            # Serijalizacija poruka u binaran oblik
    │
    ├── core/                      # 🎯 Klase za pub/sub logiku
    │   ├── PubSubEngine.h/cpp      # Centralni engine (filtriranje, dostava)
    │   ├── Publisher.h/cpp         # Izdavač poruka
    │   └── Subscriber.h/cpp        # Primač poruka
    │
    ├── utils/                     # 🛠️ Pomoćne klase i utilities
    │   ├── MessageValidator.h/cpp   # Validacija poruka (tip, vrednost)
    │   ├── MessageFormatter.h/cpp   # Formatiranje za ispis
    │   ├── CommandLineParser.h/cpp  # Parsiranje CLI parametara
    │   └── NetworkUtils.h/cpp       # Pomoć za heksadecimalne kodove
    │
    ├── DataStructures/            # Šablonske klase
    │   ├── LinkedList.h            # Ulancana lista
    │   ├── CircularBuffer.h         # Kružni bafer (FIFO)
    │   └── HashMap.h               # Hash mapa (O(1) lookup)
    │
    └── main.cpp                   # Entry point sa mode selection
```

---

## 📚 Opis Klasa

### 🎯 Core Pub/Sub Klase (`src/core/`)

#### **PubSubEngine** - Centralna Komponenta
**Lokacija:** `src/core/PubSubEngine.h/cpp`

**Odgovornost:**
- 📋 Upravljanje registracijom publisher-a i subscriber-a
- 🎯 Rutiranje poruka prema topic-ima
- 🔄 **Paralelna dostava** - svaki subscriber u drugom thread-u
- ❌ Automatsko uklanjanje mrtvih subscriber-a (svakih 5 sekundi)
- 💾 Kružni bafer za recent poruke po topic-u

**Ključne metode:**
```cpp
void publish(const Message& msg)           // Objavi poruku svim relevantnim subscriber-ima
void subscribe(int port, const std::string& topic)  // Registruj subscriber za topic
void start()                               // Pokreni engine server
void stop()                                // Ugasi engine
```

---

#### **Publisher** - Izdavač Poruka
**Lokacija:** `src/core/Publisher.h/cpp`

**Odgovornost:**
- 📤 Konekcija na PubSubEngine
- 🔄 Periodičko slanje poruka (svaki 10 sekundi)
- ✅ Validacija poruke pre slanja
- 🏷️ Setovanje publisher info-a (host + port)

**Ključne metode:**
```cpp
Publisher(int id, const std::string& host, int port, int publisher_port=0)
void start()                               // Pokreni publisher
void stop()                                // Ugasi publisher
void publish(const Message& msg)           // Pošalji pojedinačnu poruku
```

**Primer korišćenja:**
```cpp
Publisher pub(1, "localhost", 5000, 4101);
pub.start();
// U publish loop(), šalje različite tipove poruka svakih 10s
```

---

#### **Subscriber** - Primač Poruka
**Lokacija:** `src/core/Subscriber.h/cpp`

**Odgovornost:**
- 👂 Otvaranje server socket-a na specifičnoj porti
- 🎯 Registracija za željene topic-e
- 📥 Primanje poruka od Engine-a
- ✅ Filtriranje po topic-u
- 🔔 Validacija i ispis primljenih poruka

**Ključne metode:**
```cpp
Subscriber(int id, const std::vector<std::string>& topics, 
           const std::string& host, int port, int subscriber_port=0)
void start()                               // Pokreni subscriber
void stop()                                // Ugasi subscriber
```

---

### 🛠️ Pomoćne Klase (`src/utils/`)

#### **MessageValidator** - Validacija Poruka
**Lokacija:** `src/utils/MessageValidator.h/cpp`

**Odgovornost:**
- ✅ Validacija strukturnosti poruke
- 🏷️ Provera da li tip odgovara topic-u
- 📊 Provera vrednosti (ANALOG: -1000 do 10000, STATUS: OPEN/CLOSED/SWG_OPEN/SWG_CLOSED/CRB_OPEN/CRB_CLOSED)

**Ključne metode:**
```cpp
static bool validate(const Message& msg, std::string& errorMsg)
```

---

#### **MessageFormatter** - Formatiranje za Ispis
**Lokacija:** `src/utils/MessageFormatter.h/cpp`

**Odgovornost:**
- 🎨 Formatiranje poruke za čitljiv ispis
- 🏷️ Konverzija enum vrednosti u stringove

**Primer ispisa:**
```
ANALOG: Vrednost=3524.5 | Topic=Analog/MER/220
STATUS: Vrednost=CLOSED | Topic=Status/CRB/1
```

---

#### **CommandLineParser** - Parsiranje Parametara
**Lokacija:** `src/utils/CommandLineParser.h/cpp`

**Odgovornost:**
- 📋 Parsiranje CLI argumenta
- 🔍 Ekstrakcija topic-a i engine parametara

---

#### **NetworkUtils** - Mrežne Pomoćne Funkcije
**Lokacija:** `src/utils/NetworkUtils.h/cpp`

**Odgovornost:**
- 📦 Kodiranje dužine poruke (4-byte big-endian)
- 📥 Dekodiranje dužine poruke

---

### 🌐 Mrežne Klase (`src/`)

#### **TcpClient/TcpServer** (u Network.h/cpp)
- 🔌 TCP konekcija između komponenti
- 📤📥 Slanje i primanje poruka

#### **PortPool** (u Network.h/cpp)
- 🏷️ Auto-dodela portova
- Publisher: počevši od 4100
- Subscriber: počevši od 4200

#### **ConsoleHandler** (u Network.h/cpp)
- ⌨️ Capture CTRL+C, "exit" komande
- 🛑 Gracioznog isključenja programa

---

### 📦 Strukture (`src/`)

#### **Message** (u Message.h)
```cpp
struct Message {
    char topic[128]                          // "Analog/MER/220"
    char publisher_host[64]                  // "localhost"
    int publisher_port                       // 4101
    MessageType type                         // ANALOG ili STATUS
    TopicType topicType                      // MER, CRB, SWG ili OTHER
    MessageData data                         // float analogValue ili StatusValue status
    std::time_t timestamp                    // Timestamp poruke
}
```

---

### 📊 Šablonske Klase (`src/DataStructures/`)

#### **LinkedList<T>** - Ulancana Lista
- Dinamicka kolekcija sa O(n) pristupom
- Koristi se za listu subscriber-a prvo topic

#### **CircularBuffer<T>** - Kružni Bafer
- FIFO buffer sa fiksnom veličinom (50 poruka)
- Čuva recent poruke po topic-u

#### **HashMap<K,V>** - Hash Mapa
- O(1) lookup za topic-e
- Koristi se za brzo pronalaženje subscriber-a

---

## 🔄 Tok Podataka

```
1. Publisher Startuje
   └─ Konekcija na Engine (port 5000)
   └─ Setuje publisher_host, publisher_port
   └─ Šalje poruke svakih 10s

2. Engine Prima Poruku
   └─ Pronalazi sve subscriber-e za taj topic
   └─ Validira poruku
   └─ Spawn thread per subscriber
   
3. Subscriber Prima Poruku
   └─ Prima na svom socket-u
   └─ Validira tip i vrednost
   └─ Prikazuje sa publisher info-om
```

---

## 🐛 Troubleshooting

### Problem: "g++ not found"
**Rešenje:** Instalirajte MinGW sa [https://www.mingw-w64.org/](https://www.mingw-w64.org/)

### Problem: "Cannot connect to engine"
**Rešenje:** 
1. Proverite da je Engine pokrenut prvim (`--engine`)
2. Proverite port 5000 nije zauzet

### Problem: Subscriber ne prima poruke
**Rešenje:**
1. Proverite da je topic identičan (case-sensitive)
2. Proverite da Publisher šalje taj topic
3. Proverite Engine logove za greške

---

**Poslednja ažuriranja:** Februar 2026  
**Verzija:** 2.0 (Organized + Publisher Headers)
