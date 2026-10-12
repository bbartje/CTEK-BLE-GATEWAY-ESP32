#pragma once
#include <Arduino.h>

namespace ctek {
inline const char kPageStart[] PROGMEM = R"HTML(<!doctype html><html lang='nl'>
<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>CTEK gateway</title>
<style>
body{font:16px system-ui;max-width:850px;margin:2rem auto;padding:0 1rem;background:#f5f7f6;color:#172c29}
fieldset,article{border:1px solid #abc1ba;border-radius:8px;padding:1rem;margin:1rem 0}
label{display:block;margin:.6rem 0}input,select{box-sizing:border-box;width:100%;padding:.55rem}
#ui-language{width:auto}button,a.button{background:#176459;color:#fff;padding:.6rem .9rem;border:0;border-radius:4px;display:inline-block;margin:.4rem .4rem .4rem 0}
pre{white-space:pre-wrap;overflow-wrap:anywhere}[hidden]{display:none!important}small{color:#52635f}
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
  empty.forEach((slot, index) => { slot.hidden = index > 0; });
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
<body><label>Language / Taal / Sprache <select id='ui-language' aria-label='Language'>
<option value='nl'>Nederlands</option><option value='en'>English</option><option value='de'>Deutsch</option>
</select></label><h1>CTEK Battery Sense gateway</h1>)HTML";
}  // namespace ctek
