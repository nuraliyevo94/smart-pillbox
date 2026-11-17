#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="uz">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Aqlli Dori Qutisi</title>
  <style>
    :root {
      --primary: #0284c7;
      --primary-hover: #0369a1;
      --success: #16a34a;
      --warning: #eab308;
      --danger: #dc2626;
      --bg: #f8fafc;
      --card: #ffffff;
      --text: #1e293b;
      --text-muted: #64748b;
      --border: #e2e8f0;
      --radius: 16px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 16px; display: flex; justify-content: center; }
    .container { width: 100%; max-width: 520px; display: flex; flex-direction: column; gap: 16px; }
    .header {
      background: linear-gradient(135deg, #0284c7, #0f172a);
      color: #ffffff;
      padding: 22px 20px;
      border-radius: var(--radius);
      box-shadow: 0 10px 25px -5px rgba(2, 132, 199, 0.3);
      display: flex;
      align-items: center;
      gap: 16px;
    }
    .header-icon {
      width: 50px; height: 50px;
      background: rgba(255, 255, 255, 0.15);
      border-radius: 14px;
      display: flex; align-items: center; justify-content: center;
      font-size: 26px;
    }
    .header-info h1 { font-size: 19px; font-weight: 700; margin-bottom: 3px; }
    .header-info p { font-size: 12px; opacity: 0.85; }
    .card {
      background: var(--card);
      border-radius: var(--radius);
      padding: 18px;
      border: 1px solid var(--border);
      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.05);
    }
    .card-title {
      font-size: 15px;
      font-weight: 600;
      margin-bottom: 12px;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .status-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
    }
    .status-item {
      background: #f1f5f9;
      padding: 12px;
      border-radius: 12px;
      display: flex;
      flex-direction: column;
      gap: 4px;
    }
    .status-label { font-size: 11px; color: var(--text-muted); font-weight: 500; }
    .status-val { font-size: 14px; font-weight: 700; display: flex; align-items: center; gap: 6px; }
    .badge {
      display: inline-flex; align-items: center; gap: 4px;
      padding: 3px 8px; border-radius: 20px; font-size: 11px; font-weight: 600;
    }
    .badge-ok { background: #dcfce7; color: #15803d; }
    .badge-warn { background: #fef9c3; color: #a16207; }
    .badge-danger { background: #fee2e2; color: #b91c1c; }
    .battery-container { margin-top: 8px; }
    .progress-bar-bg {
      background: #e2e8f0; height: 10px; border-radius: 5px; overflow: hidden;
    }
    .progress-bar-fill {
      background: var(--success); height: 100%; width: 0%;
      transition: width 0.4s ease, background 0.4s ease;
    }
    .form-group {
      display: flex;
      flex-direction: column;
      gap: 6px;
      margin-bottom: 14px;
    }
    .form-group:last-child { margin-bottom: 0; }
    label { font-size: 13px; font-weight: 600; }
    .help-text { font-size: 11px; color: var(--text-muted); }
    input[type="text"], input[type="time"], input[type="number"], select {
      padding: 11px 13px;
      border: 1.5px solid var(--border);
      border-radius: 10px;
      font-size: 14px;
      outline: none;
      width: 100%;
      background: #fff;
    }
    input:focus { border-color: var(--primary); }
    .time-slot-list {
      display: flex;
      flex-direction: column;
      gap: 8px;
      margin-bottom: 10px;
    }
    .time-row {
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .btn-delete {
      background: #fee2e2; color: #dc2626; border: none;
      width: 40px; height: 40px; border-radius: 10px;
      font-size: 16px; cursor: pointer; display: flex; align-items: center; justify-content: center;
      flex-shrink: 0;
    }
    .btn {
      padding: 11px 16px;
      border-radius: 10px;
      border: none;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      transition: all 0.2s ease;
      width: 100%;
    }
    .btn-primary { background: var(--primary); color: #fff; }
    .btn-primary:hover { background: var(--primary-hover); }
    .btn-secondary { background: #e2e8f0; color: var(--text); }
    .btn-secondary:hover { background: #cbd5e1; }
    #toast {
      position: fixed;
      bottom: 24px; left: 50%;
      transform: translateX(-50%) translateY(100px);
      background: #0f172a;
      color: #fff;
      padding: 11px 22px;
      border-radius: 30px;
      font-size: 13px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.2);
      transition: transform 0.3s ease;
      z-index: 100;
      text-align: center;
      max-width: 90%;
    }
    #toast.show { transform: translateX(-50%) translateY(0); }
  </style>
</head>
<body>

<div class="container">
  <div class="header">
    <div class="header-icon">💊</div>
    <div class="header-info">
      <h1>Aqlli Dori Qutisi</h1>
      <p>Tibbiy nazorat & SMS xabarnoma tizimi</p>
    </div>
  </div>

  <!-- Live Status Card -->
  <div class="card">
    <div class="card-title"><span>📊</span> Real Vaqt Holati</div>
    <div class="status-grid">
      <div class="status-item">
        <span class="status-label">Quvvat (220V)</span>
        <div class="status-val" id="powerStatus"><span class="badge badge-warn">Yuklanmoqda...</span></div>
      </div>
      <div class="status-item">
        <span class="status-label">Qopqoq Holati</span>
        <div class="status-val" id="lidStatus"><span class="badge badge-warn">Yuklanmoqda...</span></div>
      </div>
      <div class="status-item">
        <span class="status-label">Qurilma Soati</span>
        <div class="status-val" id="deviceClock">--:--:--</div>
      </div>
      <div class="status-item">
        <span class="status-label">SIM800L Aloqa</span>
        <div class="status-val" id="gsmStatus"><span class="badge badge-warn">Tekshirilmoqda</span></div>
      </div>
      <div class="status-item">
        <span class="status-label">Quti Ichki Harorati</span>
        <div class="status-val" id="tempStatus">-- °C</div>
      </div>
      <div class="status-item">
        <span class="status-label">Quti Ichki Namligi</span>
        <div class="status-val" id="humidStatus">-- %</div>
      </div>
    </div>

    <div class="battery-container" style="margin-top: 12px;">
      <div style="display: flex; justify-content: space-between; margin-bottom: 5px;">
        <span class="status-label">Akkumulyator</span>
        <span class="status-label" id="batteryText" style="font-weight: 700; color: var(--text);">--%</span>
      </div>
      <div class="progress-bar-bg">
        <div class="progress-bar-fill" id="batteryBar"></div>
      </div>
      <span class="help-text" id="batteryEstimate" style="display: block; margin-top: 4px;"></span>
    </div>

    <button class="btn btn-secondary" style="margin-top: 14px;" onclick="syncTime()">
      <span>🕒</span> Telefon Vaqtiga Sinxronlash
    </button>
  </div>

  <!-- Settings Form -->
  <form id="settingsForm" onsubmit="saveSettings(event)" class="card">
    <div class="card-title"><span>⚙️</span> Sozlamalar va Jadval</div>

    <div class="form-group">
      <label for="phone1">Asosiy Telefon Raqami</label>
      <input type="text" id="phone1" placeholder="+998901234567" required>
      <span class="help-text">Format: +998901234567 (Dori va tok xabarlari boradi)</span>
    </div>

    <div class="form-group">
      <label for="phone2">Qo'shimcha Telefon (Ixtiyoriy)</label>
      <input type="text" id="phone2" placeholder="+998912345678">
      <span class="help-text">2-qarindosh yoki shifokor raqami</span>
    </div>

    <div class="form-group">
      <label>Dori Ichish Vaqtlari (Kunlik)</label>
      <span class="help-text" style="margin-bottom: 6px;">Vaqtdan ±15 daqiqa ichida quti ochilsa xabar yuboriladi</span>
      <div class="time-slot-list" id="timeSlots"></div>
      <button type="button" class="btn btn-secondary" onclick="addTimeSlot()">
        <span>➕</span> Yangi Vaqt Qo'shish
      </button>
    </div>

    <div class="form-group">
      <label for="volume">Ovoz Balandligi (5 - 30)</label>
      <div style="display: flex; align-items: center; gap: 10px;">
        <input type="range" id="volume" min="5" max="30" value="20" style="flex: 1;" oninput="document.getElementById('volVal').innerText = this.value">
        <span id="volVal" style="font-weight: 700; width: 28px; text-align: right;">20</span>
      </div>
    </div>

    <button type="submit" class="btn btn-primary" style="margin-top: 8px;">
      <span>💾</span> Sozlamalarni Saqlash
    </button>
  </form>

  <!-- Testing Card -->
  <div class="card">
    <div class="card-title"><span>🧪</span> Sinov va Diagnostika</div>
    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px;">
      <button class="btn btn-secondary" onclick="testAudio()"><span>🔊</span> Ovozni Sinash</button>
      <button class="btn btn-secondary" onclick="testSms()"><span>📩</span> Test SMS Jo'natish</button>
    </div>
  </div>
</div>

<div id="toast"></div>

<script>
  let timeList = ["08:00", "13:00", "20:00"];

  function showToast(msg) {
    const t = document.getElementById("toast");
    t.innerText = msg;
    t.classList.add("show");
    setTimeout(() => t.classList.remove("show"), 3500);
  }

  function renderTimeSlots() {
    const container = document.getElementById("timeSlots");
    container.innerHTML = "";
    timeList.forEach((time, index) => {
      const row = document.createElement("div");
      row.className = "time-row";
      row.innerHTML = `
        <input type="time" value="${time}" onchange="timeList[${index}] = this.value" required>
        <button type="button" class="btn-delete" onclick="removeTimeSlot(${index})">✕</button>
      `;
      container.appendChild(row);
    });
  }

  function addTimeSlot() {
    if (timeList.length >= 6) {
      showToast("Maksimal 6 ta vaqt kiritish mumkin");
      return;
    }
    timeList.push("12:00");
    renderTimeSlots();
  }

  function removeTimeSlot(index) {
    if (timeList.length <= 1) {
      showToast("Kamida bitta vaqt bo'lishi kerak!");
      return;
    }
    timeList.splice(index, 1);
    renderTimeSlots();
  }

  async function loadStatus() {
    try {
      const res = await fetch("/api/status");
      if (!res.ok) return;
      const data = await res.json();

      const pwr = document.getElementById("powerStatus");
      if (data.power_ac) {
        pwr.innerHTML = '<span class="badge badge-ok">⚡ 220V Ulangan</span>';
      } else {
        pwr.innerHTML = '<span class="badge badge-danger">🔋 Tok O\'chgan</span>';
      }

      const lid = document.getElementById("lidStatus");
      if (data.lid_open) {
        lid.innerHTML = '<span class="badge badge-warn">📂 Qopqoq Ochiq</span>';
      } else {
        lid.innerHTML = '<span class="badge badge-ok">🔒 Qopqoq Yopiq</span>';
      }

      document.getElementById("deviceClock").innerText = data.clock || "--:--:--";

      const gsm = document.getElementById("gsmStatus");
      if (data.gsm_ok) {
        gsm.innerHTML = `<span class="badge badge-ok">📶 Aloqa (${data.gsm_signal}%)</span>`;
      } else {
        gsm.innerHTML = '<span class="badge badge-danger">❌ SIM Aloqada Yo\'q</span>';
      }

      const bPct = Math.max(0, Math.min(100, data.bat_pct || 0));
      document.getElementById("batteryText").innerText = bPct + "% (" + (data.bat_volts || "0.0") + "V)";
      const bar = document.getElementById("batteryBar");
      bar.style.width = bPct + "%";
      bar.style.background = bPct > 35 ? "var(--success)" : (bPct > 15 ? "var(--warning)" : "var(--danger)");

      const estHours = Math.round((bPct / 100) * 45);
      document.getElementById("batteryEstimate").innerText = data.power_ac ? "Akkumulyator quvvatlanmoqda" : `Zaxira quvvat taxminan ${estHours} soatga yetadi`;

      // Temperature & Humidity (DHT22)
      const tempEl = document.getElementById("tempStatus");
      const humidEl = document.getElementById("humidStatus");
      if (data.dht_ok) {
        const tVal = parseFloat(data.temp);
        const hVal = parseFloat(data.humidity);
        tempEl.innerHTML = `${tVal}°C <span class="badge ${tVal <= 25 ? 'badge-ok' : 'badge-danger'}">${tVal <= 25 ? 'Me\'yorda' : 'Yuqori!'}</span>`;
        humidEl.innerHTML = `${hVal}% <span class="badge ${hVal <= 65 ? 'badge-ok' : 'badge-danger'}">${hVal <= 65 ? 'Quruq' : 'Nam!'}</span>`;
      } else {
        tempEl.innerHTML = `<span class="badge badge-warn">O'qilmoqda</span>`;
        humidEl.innerHTML = `<span class="badge badge-warn">O'qilmoqda</span>`;
      }

    } catch (e) {}
  }

  async function loadConfig() {
    try {
      const res = await fetch("/api/config");
      if (!res.ok) return;
      const cfg = await res.json();
      if (cfg.phone1) document.getElementById("phone1").value = cfg.phone1;
      if (cfg.phone2) document.getElementById("phone2").value = cfg.phone2;
      if (cfg.volume) {
        document.getElementById("volume").value = cfg.volume;
        document.getElementById("volVal").innerText = cfg.volume;
      }
      if (cfg.times && cfg.times.length > 0) {
        timeList = cfg.times;
        renderTimeSlots();
      }
    } catch (e) {}
  }

  async function saveSettings(e) {
    e.preventDefault();
    const phone1 = document.getElementById("phone1").value.trim();
    const phone2 = document.getElementById("phone2").value.trim();
    const volume = parseInt(document.getElementById("volume").value);

    const payload = {
      phone1: phone1,
      phone2: phone2,
      volume: volume,
      times: timeList
    };

    try {
      const res = await fetch("/api/save_config", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
      });
      if (res.ok) {
        showToast("✅ Sozlamalar muvaffaqiyatli saqlandi!");
      } else {
        showToast("❌ Saqlashda xatolik");
      }
    } catch (err) {
      showToast("❌ Qurilma bilan aloqa yo'q");
    }
  }

  async function syncTime() {
    const now = new Date();
    const payload = {
      year: now.getFullYear(),
      month: now.getMonth() + 1,
      day: now.getDate(),
      hour: now.getHours(),
      minute: now.getMinutes(),
      second: now.getSeconds()
    };
    try {
      const res = await fetch("/api/sync_time", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
      });
      if (res.ok) {
        showToast("🕒 Soat telefoningiz bilan sinxronlandi!");
        loadStatus();
      } else {
        showToast("❌ Sinxronlashda xatolik");
      }
    } catch (err) {
      showToast("❌ Ulanish xatosi");
    }
  }

  async function testAudio() {
    showToast("🔊 Ovoz eslatmasi ijro etilmoqda...");
    try { await fetch("/api/test_audio"); } catch(e) {}
  }

  async function testSms() {
    showToast("📩 Test SMS yuborilmoqda...");
    try {
      const res = await fetch("/api/test_sms");
      const r = await res.json();
      if (r.status === "ok") {
        showToast("✅ Test SMS yuborildi!");
      } else {
        showToast("❌ SIM800L xatosi: " + (r.msg || "Jo'natilmadi"));
      }
    } catch(e) {
      showToast("❌ So'rov yuborilmadi");
    }
  }

  renderTimeSlots();
  loadConfig();
  loadStatus();
  setInterval(loadStatus, 4000);
</script>
</body>
</html>
)rawliteral";
