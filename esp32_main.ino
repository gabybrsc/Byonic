
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHT_PIN       4
#define DHT_TYPE      DHT11
#define SOIL_PIN      34
#define SOUND_PIN     35
#define TRIG_PIN      18
#define ECHO_PIN      19


DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

struct Reading {
  float temp;
  float humidity;
  int   soil;        // 0-4095 raw ADC (lower = wetter on most modules)
  int   sound;       // 0-4095 raw ADC
  float distance;    // cm
  unsigned long ts;
};

#define LOG_SIZE 120
Reading log_data[LOG_SIZE];
int     log_head = 0;
int     log_count = 0;

Reading current;
String  driveMode = "stop";

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (dur == 0) return 999.0;
  return dur * 0.0343 / 2.0;
}

void sendDriveCommand(char cmd) {
  Serial2.write(cmd);
}

void handleData() {
  String json = "{";
  json += "\"temp\":"     + String(current.temp, 1)     + ",";
  json += "\"humidity\":" + String(current.humidity, 1) + ",";
  json += "\"soil\":"     + String(current.soil)        + ",";
  json += "\"sound\":"    + String(current.sound)       + ",";
  json += "\"distance\":" + String(current.distance, 1) + ",";
  json += "\"mode\":\""   + driveMode                   + "\",";
  json += "\"log\":[";
  int count = min(log_count, LOG_SIZE);
  for (int i = 0; i < count; i++) {
    int idx = (log_head - count + i + LOG_SIZE) % LOG_SIZE;
    if (i > 0) json += ",";
    json += "{";
    json += "\"t\":"    + String(log_data[idx].temp, 1)     + ",";
    json += "\"h\":"    + String(log_data[idx].humidity, 1) + ",";
    json += "\"s\":"    + String(log_data[idx].soil)        + ",";
    json += "\"snd\":"  + String(log_data[idx].sound)       + ",";
    json += "\"d\":"    + String(log_data[idx].distance, 1);
    json += "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleDrive() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    driveMode = cmd;
    if      (cmd == "forward")  sendDriveCommand('F');
    else if (cmd == "backward") sendDriveCommand('B');
    else if (cmd == "left")     sendDriveCommand('L');
    else if (cmd == "right")    sendDriveCommand('R');
    else if (cmd == "stop")     sendDriveCommand('S');
    else if (cmd == "auto")     sendDriveCommand('A');
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing cmd");
  }
}

void handleRoot() {
  String html = R"rawhtml(
<!DOCTYPE html><html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>EnvRover</title>
<style>
  :root{--bg:#0f1117;--card:#1a1d27;--border:#2a2d3a;--text:#e2e8f0;--muted:#64748b;
        --blue:#3b82f6;--teal:#14b8a6;--amber:#f59e0b;--red:#ef4444;--green:#22c55e}
  *{box-sizing:border-box;margin:0;padding:0}
  body{background:var(--bg);color:var(--text);font-family:system-ui,sans-serif;padding:16px}
  h1{font-size:18px;font-weight:500;margin-bottom:16px;color:var(--blue)}
  .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:10px;margin-bottom:16px}
  .card{background:var(--card);border:1px solid var(--border);border-radius:10px;padding:14px}
  .card .label{font-size:11px;color:var(--muted);margin-bottom:4px;text-transform:uppercase;letter-spacing:.05em}
  .card .val{font-size:26px;font-weight:500}
  .card .unit{font-size:12px;color:var(--muted);margin-left:2px}
  .controls{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:16px}
  .btn{padding:10px 18px;border:none;border-radius:8px;font-size:14px;cursor:pointer;
       background:var(--card);color:var(--text);border:1px solid var(--border);transition:background .15s}
  .btn:hover{background:#252836}
  .btn.active{background:var(--blue);border-color:var(--blue);color:#fff}
  .btn.danger{background:var(--red);border-color:var(--red);color:#fff}
  canvas{width:100%;height:120px;background:var(--card);border:1px solid var(--border);border-radius:10px}
  .chart-row{margin-bottom:10px}
  .chart-label{font-size:11px;color:var(--muted);margin-bottom:4px}
  .alert{background:#7f1d1d;border:1px solid var(--red);border-radius:8px;padding:10px 14px;
         margin-bottom:12px;font-size:13px;display:none}
  .alert.show{display:block}
  .status{font-size:12px;color:var(--muted);margin-top:12px}
</style>
</head>
<body>
<h1>EnvRover dashboard</h1>
<div id="alert" class="alert">Obstacle detected — auto-stopped</div>
<div class="grid">
  <div class="card">
    <div class="label">Temperature</div>
    <div class="val" id="temp">--<span class="unit">°C</span></div>
  </div>
  <div class="card">
    <div class="label">Humidity</div>
    <div class="val" id="hum">--<span class="unit">%</span></div>
  </div>
  <div class="card">
    <div class="label">Soil moisture</div>
    <div class="val" id="soil" style="font-size:20px">--</div>
  </div>
  <div class="card">
    <div class="label">Noise level</div>
    <div class="val" id="snd" style="font-size:20px">--</div>
  </div>
  <div class="card">
    <div class="label">Distance</div>
    <div class="val" id="dist">--<span class="unit">cm</span></div>
  </div>
</div>

<div class="controls">
  <button class="btn" onclick="drive('forward')">Forward</button>
  <button class="btn" onclick="drive('backward')">Back</button>
  <button class="btn" onclick="drive('left')">Left</button>
  <button class="btn" onclick="drive('right')">Right</button>
  <button class="btn danger" onclick="drive('stop')">Stop</button>
  <button class="btn" id="btn-auto" onclick="drive('auto')">Auto mode</button>
</div>

<div class="chart-row">
  <div class="chart-label">Temperature (last 2 min)</div>
  <canvas id="c-temp" height="120"></canvas>
</div>
<div class="chart-row">
  <div class="chart-label">Humidity (last 2 min)</div>
  <canvas id="c-hum" height="120"></canvas>
</div>
<div class="chart-row">
  <div class="chart-label">Noise (last 2 min)</div>
  <canvas id="c-snd" height="120"></canvas>
</div>

<div class="status" id="status">Connecting...</div>

<script>
let lastMode = '';

function drive(cmd) {
  fetch('/drive?cmd=' + cmd);
  document.querySelectorAll('.btn').forEach(b => b.classList.remove('active'));
  if(cmd !== 'stop') {
    document.getElementById('btn-auto').classList.toggle('active', cmd === 'auto');
  }
}

function drawChart(canvasId, data, color, minVal, maxVal) {
  const canvas = document.getElementById(canvasId);
  const ctx = canvas.getContext('2d');
  canvas.width = canvas.offsetWidth;
  canvas.height = 120;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (!data || data.length < 2) return;
  const pad = 8;
  const w = canvas.width - pad*2;
  const h = canvas.height - pad*2;
  const range = maxVal - minVal || 1;
  ctx.strokeStyle = color;
  ctx.lineWidth = 1.5;
  ctx.beginPath();
  data.forEach((v, i) => {
    const x = pad + (i / (data.length - 1)) * w;
    const y = pad + h - ((v - minVal) / range) * h;
    i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
  });
  ctx.stroke();
  ctx.fillStyle = color + '22';
  ctx.lineTo(pad + w, pad + h);
  ctx.lineTo(pad, pad + h);
  ctx.closePath();
  ctx.fill();
}

async function poll() {
  try {
    const r = await fetch('/data');
    const d = await r.json();
    document.getElementById('temp').innerHTML = d.temp + '<span class="unit">°C</span>';
    document.getElementById('hum').innerHTML  = d.humidity + '<span class="unit">%</span>';
    document.getElementById('soil').textContent = d.soil;
    document.getElementById('snd').textContent  = d.sound;
    document.getElementById('dist').innerHTML = d.distance + '<span class="unit">cm</span>';
    document.getElementById('status').textContent = 'Updated ' + new Date().toLocaleTimeString() + ' · Mode: ' + d.mode;
    document.getElementById('alert').classList.toggle('show', d.distance < 15 && d.mode !== 'stop');

    if (d.log && d.log.length > 1) {
      drawChart('c-temp', d.log.map(x => x.t),   '#3b82f6', 10, 45);
      drawChart('c-hum',  d.log.map(x => x.h),   '#14b8a6', 0, 100);
      drawChart('c-snd',  d.log.map(x => x.snd), '#f59e0b', 0, 4095);
    }
  } catch(e) {
    document.getElementById('status').textContent = 'Connection lost — retrying...';
  }
}

poll();
setInterval(poll, 1000);

document.addEventListener('keydown', e => {
  if(e.key === 'ArrowUp')    drive('forward');
  if(e.key === 'ArrowDown')  drive('backward');
  if(e.key === 'ArrowLeft')  drive('left');
  if(e.key === 'ArrowRight') drive('right');
  if(e.key === ' ')          drive('stop');
});
</script>
</body></html>
)rawhtml";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, -1, 17); // TX only to Arduino

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();

WiFi.softAP("EnvRover", "rover1234");
Serial.println("AP started — connect to EnvRover, open 192.168.4.1");

  server.on("/",      handleRoot);
  server.on("/data",  handleData);
  server.on("/drive", handleDrive);
  server.begin();
}

unsigned long lastSample = 0;
const unsigned long SAMPLE_MS = 2000; // read sensors every 2 seconds

void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) current.temp     = t;
    if (!isnan(h)) current.humidity = h;
    current.soil     = analogRead(SOIL_PIN);
    current.sound    = analogRead(SOUND_PIN);
    current.distance = readDistance();
    current.ts       = now;

    if (current.distance < 15.0 && driveMode == "auto") {
      sendDriveCommand('S');
      driveMode = "stop";
    }

    log_data[log_head] = current;
    log_head = (log_head + 1) % LOG_SIZE;
    if (log_count < LOG_SIZE) log_count++;

  }
}
