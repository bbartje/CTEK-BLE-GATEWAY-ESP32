#pragma once
#include <Arduino.h>

namespace ctek {
inline const char kPageStart[] PROGMEM = R"HTML(<!doctype html><html lang='nl'>
<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>CTEK gateway</title>
<style>
*{box-sizing:border-box}html{background:#f4ecdc}
body{font:16px/1.5 system-ui,sans-serif;max-width:960px;margin:1.25rem auto;padding:0 1rem 2rem;color:#203e3d}
a{color:#944021;text-underline-offset:.2em}a:hover{color:#203e3d}
button,input,select{font:inherit}button,a.button,summary{touch-action:manipulation}
button,a.button{background:#185853;color:#fffaf0;padding:.65rem 1rem;border:1px solid #185853;border-radius:10px;display:inline-block;margin:.35rem .35rem .35rem 0;cursor:pointer;text-decoration:none;font-weight:650;line-height:1.35}
button:hover,a.button:hover{background:#244542;border-color:#244542}
:focus-visible{outline:3px solid #944021;outline-offset:4px}
label{display:block;margin:.65rem 0;min-width:0}input,select{width:100%;padding:.6rem;border:1px solid #80928a;border-radius:7px;background:#fffdf7;color:#203e3d}
input[type=checkbox]{width:auto;accent-color:#185853}
fieldset,article,.gateway-config{border:1px solid #b8bb9e;border-radius:16px;background:#fffaf0;padding:1.1rem;margin:1rem 0}
fieldset{min-width:0}legend{font-weight:650;padding:0 .4rem}
pre{white-space:pre-wrap;overflow-wrap:anywhere;background:#f4ecdc;padding:.85rem;border-radius:8px}[hidden]{display:none!important}small{color:#52645f}
.page-top{display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:.5rem 1rem;font-size:.85rem;margin-bottom:1rem}
.page-top label{margin:0}.page-top select{width:auto;padding:.3rem}.repository-link{font-weight:650}
.hero{position:relative;overflow:hidden;background:#203e3d;color:#fffaf0;border-radius:22px;padding:1.5rem 1.7rem;border-bottom:7px solid #d6793d}
.hero:before{content:'';position:absolute;top:0;left:0;width:100%;height:6px;background:linear-gradient(90deg,#d6793d 0 35%,#e8b75b 35% 65%,#74a39a 65%)}
.hero h1{font-size:clamp(1.55rem,4vw,2.3rem);line-height:1.15;letter-spacing:-.035em;margin:.25rem 0 .7rem;max-width:24ch}
.hero p{margin:0;color:#e9e4ce}.hero small{color:#e8b75b;text-transform:uppercase;letter-spacing:.12em;font-size:.75rem;font-weight:700}
.gateway-status{font-size:.9rem;padding:.8rem 0}.gateway-status p{margin:.25rem 0;overflow-wrap:anywhere}
.gateway-actions{position:sticky;top:.5rem;z-index:2;display:flex;align-items:center;flex-wrap:wrap;gap:.4rem;padding:.55rem;background:#fffaf0;border:1px solid #b8bb9e;border-radius:14px;box-shadow:0 4px 0 #203e3d12}
.gateway-actions form{margin:0;display:flex}.gateway-actions button,.gateway-actions a.button{margin:0;font-size:.85rem;padding:.6rem .8rem}
.gateway-actions .secondary{background:#f4ecdc;color:#203e3d;border-color:#b8bb9e}.gateway-actions .secondary:hover{background:#e9e0cc}
.gateway-config>summary{font-weight:700}.gateway-config>p{color:#52645f;font-size:.9rem}
.gateway-config fieldset{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:.25rem 1rem;border:0;padding:0;margin:.75rem 0}
.gateway-config fieldset label:last-child{grid-column:1/-1}
h2{font-size:1.3rem;letter-spacing:-.025em;margin:1.5rem 0 .5rem}
.sensor-slot{border-left:5px solid #185853;box-shadow:0 3px 0 #203e3d0d}
.sensor-slot:nth-of-type(2n){border-left-color:#ac431e}
.sensor-slot h3{margin:0 0 .85rem;font-size:1.05rem}
.sensor-reading{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:.75rem 1.5rem}
.sensor-caption{display:block;color:#52645f;font-size:.8rem;margin-bottom:.2rem}
.sensor-voltage{display:block;font:700 2rem/1.2 ui-monospace,monospace;letter-spacing:-.04em;overflow-wrap:anywhere}
.sensor-rssi{display:block;font-size:1.2rem}
.sensor-time{line-height:1.5}.sensor-freshness{display:block;margin-top:.2rem}
.sensor-settings{margin-top:.9rem;border-top:1px dashed #b8bb9e;padding-top:.45rem}summary{cursor:pointer;min-height:44px;padding:.5rem 0}summary::marker{color:#944021}
#add-sensor{font-size:1.4rem;min-width:48px}.page-note{display:block;margin:1rem 0}
@media(max-width:600px){body{padding:0 .75rem 1.5rem;margin:.75rem auto}.hero{padding:1.3rem 1.1rem}.gateway-config fieldset{grid-template-columns:1fr}.gateway-actions{top:0;gap:.35rem}.gateway-actions form{flex:1}.gateway-actions button{width:100%}.sensor-reading>div{flex:1 1 125px}.sensor-time{flex-basis:100%!important}.sensor-voltage{font-size:1.75rem}}
</style>
<script>
window.addEventListener('DOMContentLoaded', () => {
  const selector = document.getElementById('ui-language');
  let saved = 'nl';
  try { saved = localStorage.getItem('ctek-language') || 'nl'; } catch {}
  if (!['nl', 'en', 'de'].includes(saved)) saved = 'nl';
  function applyLanguage(language) {
    document.documentElement.lang = language;
    document.querySelectorAll('[data-nl]').forEach(node => {
      if (node.dataset[language]) node.textContent = node.dataset[language];
    });
  }
  selector.value = saved;
  applyLanguage(saved);
  selector.addEventListener('change', () => {
    try { localStorage.setItem('ctek-language', selector.value); } catch {}
    applyLanguage(selector.value);
  });
  const slots = [...document.querySelectorAll('.sensor-slot')];
  const empty = slots.filter(slot => slot.dataset.configured === '0');
  empty.forEach(slot => { slot.hidden = true; });
  const add = document.getElementById('add-sensor');
  if (add) {
    const refresh = () => { add.hidden = !empty.some(slot => slot.hidden); };
    add.addEventListener('click', () => {
      const next = empty.find(slot => slot.hidden);
      if (next) next.hidden = false;
      refresh();
    });
    refresh();
  }
});
</script>
<body><div class='page-top'><a class='repository-link' href='https://github.com/bbartje/CTEK-BLE-GATEWAY-ESP32' target='_blank' rel='noopener noreferrer'>BartjeB / GitHub &#8599;</a><label>Language / Taal / Sprache <select id='ui-language' aria-label='Language'>
<option value='nl'>Nederlands</option><option value='en'>English</option><option value='de'>Deutsch</option>
</select></label></div><header class='hero'><small>BLE &middot; ESP32 &middot; Home Assistant</small><h1>CTEK Battery Sense gateway</h1>
<p data-nl='Je accu in beeld. Klaar voor de volgende rit.' data-en='Your battery at a glance. Ready for the next trip.' data-de='Ihre Batterie im Blick. Bereit für die nächste Fahrt.'>Je accu in beeld. Klaar voor de volgende rit.</p></header>)HTML";

// Shared markup: browser tests exercise the actual controls served by the board.
inline const char kGatewayActions[] PROGMEM = R"HTML(<nav class='gateway-actions' aria-label='Gateway'>
<a class='button secondary' href='/'><span data-nl='Vernieuwen' data-en='Refresh' data-de='Aktualisieren'>Vernieuwen</span></a>
<form method='post' action='/scan'><input type='hidden' name='csrf' value='{{csrf}}'><button><span data-nl='Bluetooth zoeken (6 s)' data-en='Scan Bluetooth (6 s)' data-de='Bluetooth suchen (6 s)'>Bluetooth zoeken (6 s)</span></button></form>
<a class='button secondary' href='/diagnostics'><span data-nl='Diagnostics downloaden' data-en='Download diagnostics' data-de='Diagnose herunterladen'>Diagnostics downloaden</span></a>
<form method='post' action='/ota-mode'><input type='hidden' name='csrf' value='{{csrf}}'><button><span data-nl='Firmware bijwerken' data-en='Update firmware' data-de='Firmware aktualisieren'>Firmware bijwerken</span></button></form>
</nav>)HTML";
inline const char kConfigurationStart[] PROGMEM = R"HTML(<details class='gateway-config' id='gateway-config'>
<summary><span data-nl='Gateway-instellingen' data-en='Gateway settings' data-de='Gateway-Einstellungen'>Gateway-instellingen</span></summary>
<p data-nl='Open om wifi, MQTT, meetinterval of je beheerwachtwoord te wijzigen.' data-en='Open to change Wi-Fi, MQTT, polling interval or your admin password.' data-de='Öffnen, um WLAN, MQTT, Abfrageintervall oder Ihr Admin-Passwort zu ändern.'>Open om wifi, MQTT, meetinterval of je beheerwachtwoord te wijzigen.</p>)HTML";
inline const char kConfigurationEnd[] PROGMEM = R"HTML(</details>)HTML";
}  // namespace ctek
