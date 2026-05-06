#include "HttpServer.h"
#include "../core/PubSubEngine.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstring>

// ============================================================
//  Constructor / Destructor
// ============================================================

HttpServer::HttpServer(PubSubEngine* eng, int httpPort)
    : listenSocket(INVALID_SOCKET), port(httpPort), running(false), engine(eng)
{
    // Winsock je vec inicijaliziran od strane TcpClient/TcpServer,
    // ali WSAStartup je siguran za visestruko pozivanje
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

HttpServer::~HttpServer() {
    stop();
}

// ============================================================
//  Start / Stop
// ============================================================

void HttpServer::start() {
    if (running) return;

    listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "[HttpServer] Greska: nije moguce kreirati socket" << std::endl;
        return;
    }

    // Dozvoli ponovnu upotrebu porta
    int reuse = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Prima konekcije sa bilo koje adrese
    addr.sin_port        = htons(port);

    if (::bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Greska: bind nije uspio na portu " << port << std::endl;
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return;
    }

    if (::listen(listenSocket, 10) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Greska: listen nije uspio" << std::endl;
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
        return;
    }

    running = true;
    serverThread = std::thread(&HttpServer::serveLoop, this);
    serverThread.detach();

    std::cout << "[HttpServer] Dashboard dostupan na: http://localhost:" << port << std::endl;
}

void HttpServer::stop() {
    if (running) {
        running = false;
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
    }
}

// ============================================================
//  Serve loop - prihvata HTTP konekcije
// ============================================================

void HttpServer::serveLoop() {
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (running) {
        SOCKET client = ::accept(listenSocket,
                                 (struct sockaddr*)&clientAddr,
                                 &clientAddrLen);
        if (client == INVALID_SOCKET) {
            if (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            break;
        }

        // Svaki HTTP zahtjev obradi u posebnom thread-u
        std::thread([this, client]() {
            handleClient(client);
        }).detach();
    }
}

// ============================================================
//  Handle client - parsira HTTP request, salje odgovor
// ============================================================

void HttpServer::handleClient(SOCKET client) {
    // Timeout za citanje zahtjeva
    DWORD timeout = 3000;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // Citaj HTTP zahtjev
    char buffer[4096] = {0};
    int received = ::recv(client, buffer, sizeof(buffer) - 1, 0);

    if (received <= 0) {
        closesocket(client);
        return;
    }
    buffer[received] = '\0';

    // Parsiraj putanju iz prve linije: "GET /path HTTP/1.1"
    std::string request(buffer);
    std::string path = "/";

    size_t spacePos = request.find(' ');
    if (spacePos != std::string::npos) {
        size_t pathEnd = request.find(' ', spacePos + 1);
        if (pathEnd != std::string::npos) {
            path = request.substr(spacePos + 1, pathEnd - spacePos - 1);
        }
    }

    // Odredi sadrzaj odgovora
    std::string body;
    std::string contentType;

    if (path == "/api/stats") {
        body        = engine->getStatsJson();
        contentType = "application/json";
    } else {
        // Sve ostale putanje serviraju HTML dashboard
        body        = getHtmlPage();
        contentType = "text/html; charset=utf-8";
    }

    // Sastavi HTTP odgovor
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: "   << contentType  << "\r\n";
    response << "Content-Length: " << body.size()  << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    std::string responseStr = response.str();
    ::send(client, responseStr.c_str(), (int)responseStr.size(), 0);
    closesocket(client);
}

// ============================================================
//  HTML Dashboard (ugradjeno kao raw string literal)
// ============================================================

std::string HttpServer::getHtmlPage() {
    return R"html(<!DOCTYPE html>
<html lang="sr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>PubSub Engine Monitor</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
<style>
*{box-sizing:border-box;margin:0;padding:0;}
body{background:#111318;color:#cdd6f4;font-family:'Segoe UI',system-ui,monospace;padding:0;}
header{background:#181c24;border-bottom:1px solid #2a2d3a;padding:14px 28px;display:flex;align-items:center;justify-content:space-between;}
.logo{display:flex;align-items:center;gap:10px;}
.logo h1{color:#89b4fa;font-size:16px;font-weight:600;letter-spacing:0.5px;}
.logo .sub{color:#585b70;font-size:11px;margin-top:2px;}
.pulse{width:8px;height:8px;border-radius:50%;background:#a6e3a1;display:inline-block;animation:pulse 2s infinite;}
@keyframes pulse{0%,100%{opacity:1;box-shadow:0 0 0 0 rgba(166,227,161,0.4);}50%{opacity:0.7;box-shadow:0 0 0 5px rgba(166,227,161,0);}}
.hdr-right{color:#585b70;font-size:11px;text-align:right;}
.hdr-right span{color:#a6e3a1;margin-left:6px;}

.main{padding:20px 28px;}

/* Kartice */
.cards{display:grid;grid-template-columns:repeat(4,1fr);gap:12px;margin-bottom:20px;}
@media(max-width:800px){.cards{grid-template-columns:repeat(2,1fr);}}
.card{background:#181c24;border:1px solid #2a2d3a;border-radius:8px;padding:16px 20px;position:relative;overflow:hidden;}
.card::before{content:'';position:absolute;top:0;left:0;right:0;height:2px;}
.card.blue::before{background:#89b4fa;}
.card.green::before{background:#a6e3a1;}
.card.yellow::before{background:#f9e2af;}
.card.mauve::before{background:#cba6f7;}
.card .val{font-size:30px;font-weight:700;font-variant-numeric:tabular-nums;margin-bottom:4px;}
.card.blue .val{color:#89b4fa;}
.card.green .val{color:#a6e3a1;}
.card.yellow .val{color:#f9e2af;}
.card.mauve .val{color:#cba6f7;}
.card .lbl{color:#585b70;font-size:10px;text-transform:uppercase;letter-spacing:1.2px;}

/* Grafikon i status */
.row-top{display:grid;grid-template-columns:1fr 340px;gap:12px;margin-bottom:12px;}
@media(max-width:900px){.row-top{grid-template-columns:1fr;}}
.box{background:#181c24;border:1px solid #2a2d3a;border-radius:8px;padding:18px;}
.box-title{color:#585b70;font-size:10px;text-transform:uppercase;letter-spacing:1.2px;margin-bottom:14px;display:flex;align-items:center;gap:8px;}
.box-title::after{content:'';flex:1;height:1px;background:#2a2d3a;}

/* Status indikatori */
.status-grid{display:flex;flex-direction:column;gap:10px;}
.status-item{background:#1e2030;border:1px solid #2a2d3a;border-radius:6px;padding:12px 14px;display:flex;align-items:center;justify-content:space-between;}
.status-topic{font-size:12px;color:#a6adc8;font-family:monospace;}
.status-badge{font-size:11px;font-weight:700;padding:3px 10px;border-radius:4px;letter-spacing:0.5px;}
.badge-open{background:#1e3a1e;color:#a6e3a1;border:1px solid #2d5a2d;}
.badge-closed{background:#3a1e1e;color:#f38ba8;border:1px solid #5a2d2d;}
.badge-unknown{background:#2a2d3a;color:#585b70;border:1px solid #3a3d4a;}

/* Alarm panel */
.alarm-bar{display:none;background:#2a0a0a;border:1px solid #f38ba8;border-radius:8px;padding:12px 18px;margin-bottom:12px;}
.alarm-bar.active{display:flex;align-items:flex-start;gap:12px;}
.alarm-icon{font-size:20px;flex-shrink:0;}
.alarm-title{color:#f38ba8;font-weight:700;font-size:13px;margin-bottom:6px;}
.alarm-list{list-style:none;display:flex;flex-direction:column;gap:4px;}
.alarm-list li{color:#f5c2c2;font-size:12px;font-family:monospace;}

/* Tabela poruka */
.row-bot{display:grid;grid-template-columns:1fr 1fr;gap:12px;}
@media(max-width:800px){.row-bot{grid-template-columns:1fr;}}
table{width:100%;border-collapse:collapse;}
th{color:#45475a;padding:6px 10px;text-align:left;font-size:10px;text-transform:uppercase;letter-spacing:0.8px;}
td{padding:8px 10px;border-top:1px solid #1e2030;font-size:12px;font-family:monospace;color:#a6adc8;}
tr:hover td{background:#1e2030;}
.tag{display:inline-block;padding:1px 6px;border-radius:3px;font-size:10px;font-weight:700;letter-spacing:0.5px;}
.tag.a{background:#1e3052;color:#89b4fa;}
.tag.s{background:#1e3a1e;color:#a6e3a1;}
.val-analog{color:#cdd6f4;}
.val-open{color:#a6e3a1;}
.val-closed{color:#f38ba8;}
.empty{color:#313244;text-align:center;padding:24px;font-size:12px;}
</style>
</head>
<body>

<header>
  <div class="logo">
    <span class="pulse"></span>
    <div>
      <h1>PubSub Engine Monitor</h1>
    </div>
  </div>
  <div class="hdr-right">
    Poslednje osvežavanje<br><span id="upd">—</span>
  </div>
</header>

<div class="main">
  <div class="cards">
    <div class="card blue">
      <div class="val" id="c-up">—</div>
      <div class="lbl">Uptime</div>
    </div>
    <div class="card green">
      <div class="val" id="c-msg">—</div>
      <div class="lbl">Ukupno poruka</div>
    </div>
    <div class="card yellow">
      <div class="val" id="c-top">—</div>
      <div class="lbl">Aktivnih topika</div>
    </div>
    <div class="card mauve">
      <div class="val" id="c-sub">—</div>
      <div class="lbl">Pretplatnika</div>
    </div>
  </div>

  <!-- Alarm panel — prikazuje se samo kada postoji prekoracenje -->
  <div class="alarm-bar" id="alarm-bar">
    <div class="alarm-icon">&#9888;</div>
    <div>
      <div class="alarm-title">ALARM — Prekoracenje dozvoljene granice</div>
      <ul class="alarm-list" id="alarm-list"></ul>
    </div>
  </div>

  <div class="row-top">
    <!-- Linijski grafikon analognih vrednosti -->
    <div class="box">
      <div class="box-title">Analogne vrednosti u realnom vremenu (poslednjih 20 merenja)</div>
      <canvas id="chart" height="110"></canvas>
    </div>

    <!-- Status indikatori -->
    <div class="box">
      <div class="box-title">Trenutni status digitalnih signala</div>
      <div class="status-grid" id="status-grid">
        <div class="empty">Nema status topika</div>
      </div>
    </div>
  </div>

  <div class="row-bot">
    <!-- Tabela topika -->
    <div class="box">
      <div class="box-title">Aktivni topici</div>
      <table>
        <thead>
          <tr>
            <th>Topic</th><th>Sub</th><th>Poruke</th><th>Zadnja vrednost</th><th>Vreme</th>
          </tr>
        </thead>
        <tbody id="tb-t"><tr><td colspan="5" class="empty">Nema podataka</td></tr></tbody>
      </table>
    </div>

    <!-- Tabela poslednjih poruka -->
    <div class="box">
      <div class="box-title">Log poslednjih poruka</div>
      <table>
        <thead>
          <tr><th>Vreme</th><th>Topic</th><th>Tip</th><th>Vrednost</th></tr>
        </thead>
        <tbody id="tb-r"><tr><td colspan="4" class="empty">Jos nema poruka</td></tr></tbody>
      </table>
    </div>
  </div>
</div>

<script>
// ---- Lokalni buffer za linijski grafikon ----
// key: topic name, value: [{time, value}]
const analogHistory = {};
const COLORS = ['#89b4fa','#a6e3a1','#f9e2af','#cba6f7','#f38ba8','#94e2d5'];
let colorIdx = 0;
const topicColors = {};
const hiddenTopics = new Set();  // Trajno cuva koje linije su sakrivene
let lineChart = null;

function getColor(topic) {
  if (!topicColors[topic]) {
    topicColors[topic] = COLORS[colorIdx % COLORS.length];
    colorIdx++;
  }
  return topicColors[topic];
}

function fmtUptime(s) {
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  return (h > 0 ? h + 'h ' : '') + (m > 0 ? m + 'm ' : '') + sec + 's';
}

function buildLineChart(analogTopics, newMsgs) {
  // Dodaj nove analog poruke u history
  for (const m of newMsgs) {
    if (m.type !== 'ANALOG') continue;
    if (!analogHistory[m.topic]) analogHistory[m.topic] = [];
    const arr = analogHistory[m.topic];
    if (arr.length === 0 || arr[arr.length-1].time !== m.time) {
      arr.push({ time: m.time, value: parseFloat(m.value) });
      if (arr.length > 20) arr.shift();
    }
  }

  const allTimes = [...new Set(
    Object.values(analogHistory).flat().map(p => p.time)
  )].sort();

  if (allTimes.length === 0) return;

  const datasets = Object.entries(analogHistory).map(([topic, pts]) => {
    const ptMap = {};
    pts.forEach(p => ptMap[p.time] = p.value);
    return {
      label: topic,
      data: allTimes.map(t => ptMap[t] ?? null),
      borderColor: getColor(topic),
      backgroundColor: 'transparent',
      borderWidth: 2,
      pointRadius: 3,
      pointBackgroundColor: getColor(topic),
      tension: 0.3,
      spanGaps: true,
      hidden: hiddenTopics.has(topic)  // Primeni sacuvano stanje
    };
  });

  const ctx = document.getElementById('chart').getContext('2d');
  if (lineChart) {
    lineChart.data.labels   = allTimes;
    lineChart.data.datasets = datasets;
    lineChart.update('none');
    // Mora da se postavi na meta NAKON update-a jer Chart.js resetuje meta pri zameni datasets
    datasets.forEach((ds, i) => {
      lineChart.getDatasetMeta(i).hidden = hiddenTopics.has(ds.label);
    });
    lineChart.update('none');
  } else {
    lineChart = new Chart(ctx, {
      type: 'line',
      data: { labels: allTimes, datasets },
      options: {
        responsive: true,
        animation: false,
        plugins: {
          legend: {
            display: true,
            labels: { color: '#a6adc8', font: { size: 11 }, boxWidth: 12, padding: 14 },
            onClick: function(e, legendItem, legend) {
              const topic = legendItem.text;
              // Sacuvaj stanje — ako je bio skriven, prikazi i obrnuto
              if (hiddenTopics.has(topic)) {
                hiddenTopics.delete(topic);
              } else {
                hiddenTopics.add(topic);
              }
              // Standardni Chart.js toggle
              const index = legendItem.datasetIndex;
              const meta  = legend.chart.getDatasetMeta(index);
              meta.hidden = hiddenTopics.has(topic);
              legend.chart.update();
            }
          }
        },
        scales: {
          x: {
            ticks: { color: '#45475a', font: { size: 10 }, maxTicksLimit: 8 },
            grid: { color: '#1e2030' }
          },
          y: {
            ticks: { color: '#45475a', font: { size: 10 } },
            grid: { color: '#1e2030' }
          }
        }
      }
    });
  }
}

function buildStatusGrid(topics) {
  const statusTopics = topics.filter(t => t.type === 'STATUS');
  const grid = document.getElementById('status-grid');
  if (statusTopics.length === 0) {
    grid.innerHTML = '<div class="empty">Nema status topika</div>';
    return;
  }
  grid.innerHTML = statusTopics.map(t => {
    const isOpen = t.lastValue.includes('OPEN');
    const cls    = t.lastValue === '' ? 'badge-unknown' : (isOpen ? 'badge-open' : 'badge-closed');
    const lbl    = t.lastValue === '' ? 'N/A' : t.lastValue;
    return `<div class="status-item">
      <div>
        <div class="status-topic">${t.name}</div>
      </div>
      <span class="status-badge ${cls}">${lbl}</span>
    </div>`;
  }).join('');
}

// Pamti poslednje vidjene poruke da ne bi duplikate dodavali u history
let lastSeenMsgs = [];

function checkAlarms(topics) {
  const alarms = [];
  for (const t of topics) {
    if (t.type !== 'ANALOG' || !t.lastValue || t.lastValue === '—') continue;

    // Izvuci nominalnu vrednost iz naziva topica (npr. "Analog/MER/220" -> 220)
    const parts = t.name.split('/');
    const nominal = parseFloat(parts[parts.length - 1]);
    if (isNaN(nominal)) continue;

    const current = parseFloat(t.lastValue);
    const deviation = Math.abs(current - nominal);
    if (deviation > 10) {
      const dir = current > nominal ? 'visoka' : 'niska';
      alarms.push(`${t.name}: ${current.toFixed(2)} (nominalna ${nominal}, odstupanje ${deviation.toFixed(2)} — vrednost ${dir})`);
    }
  }

  const bar  = document.getElementById('alarm-bar');
  const list = document.getElementById('alarm-list');
  if (alarms.length > 0) {
    bar.classList.add('active');
    list.innerHTML = alarms.map(a => `<li>&#8226; ${a}</li>`).join('');
  } else {
    bar.classList.remove('active');
    list.innerHTML = '';
  }
}

async function refresh() {
  try {
    const r = await fetch('/api/stats');
    if (!r.ok) throw new Error('HTTP ' + r.status);
    const d = await r.json();

    document.getElementById('c-up').textContent  = fmtUptime(d.uptime || 0);
    document.getElementById('c-msg').textContent = d.totalMessages || 0;
    document.getElementById('c-top').textContent = d.totalTopics || 0;
    document.getElementById('c-sub').textContent = d.totalSubscribers || 0;
    document.getElementById('upd').textContent   = new Date().toLocaleTimeString();

    const topics  = d.topics  || [];
    const recent  = d.recentMessages || [];

    // Nove poruke koje jos nismo obradili
    const newMsgs = recent.filter(m =>
      !lastSeenMsgs.some(p => p.time === m.time && p.topic === m.topic && p.value === m.value)
    );
    lastSeenMsgs = recent.slice();

    buildLineChart(topics, newMsgs);
    buildStatusGrid(topics);
    checkAlarms(topics);

    // Tabela topika
    const tbT = document.getElementById('tb-t');
    if (topics.length === 0) {
      tbT.innerHTML = '<tr><td colspan="5" class="empty">Nema aktivnih topika</td></tr>';
    } else {
      tbT.innerHTML = topics.map(t => {
        const isOpen = t.lastValue.includes('OPEN');
        const cls = t.type==='ANALOG' ? 'val-analog' : (isOpen ? 'val-open' : 'val-closed');
        return `<tr>
          <td>${t.name}</td>
          <td>${t.subscribers}</td>
          <td>${t.messages}</td>
          <td class="${cls}">${t.lastValue||'—'}</td>
          <td>${t.lastTime||'—'}</td>
        </tr>`;
      }).join('');
    }

    // Log poruka — najnovije gore
    const tbR = document.getElementById('tb-r');
    const sorted = recent.slice().reverse();
    if (sorted.length === 0) {
      tbR.innerHTML = '<tr><td colspan="4" class="empty">Jos nema poruka</td></tr>';
    } else {
      tbR.innerHTML = sorted.map(m => {
        const isOpen = m.value.includes('OPEN');
        const cls = m.type==='ANALOG' ? 'val-analog' : (isOpen ? 'val-open' : 'val-closed');
        return `<tr>
          <td>${m.time}</td>
          <td>${m.topic}</td>
          <td><span class="tag ${m.type==='ANALOG'?'a':'s'}">${m.type}</span></td>
          <td class="${cls}">${m.value}</td>
        </tr>`;
      }).join('');
    }

  } catch(e) {
    document.getElementById('upd').textContent = 'Greska: ' + e.message;
  }
}

refresh();
setInterval(refresh, 2000);
</script>
</body>
</html>)html";
}