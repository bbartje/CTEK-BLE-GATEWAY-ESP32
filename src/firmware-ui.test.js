import { readFileSync } from 'node:fs';
import { JSDOM } from 'jsdom';

// Execute the script served by the ESP32, with no copied implementation.
const source = readFileSync('firmware/include/web_ui.h', 'utf8');
const pageStart = source.match(/R"HTML\(([\s\S]*?)\)HTML"/)[1];
const pages = [];

async function openPage(configured = [true, false, false, false], options = {}) {
  const slots = configured.map(value => `<article class="sensor-slot" data-configured="${value ? 1 : 0}"></article>`).join('');
  const page = new JSDOM(`${pageStart}
    <span id="label" data-nl="Opslaan" data-en="Save" data-de="Speichern">Opslaan</span>
    ${slots}<button id="add-sensor" type="button">+</button></body></html>`, {
    url: 'http://192.168.4.1',
    runScripts: 'dangerously',
    beforeParse(window) {
      if (options.storageBlocked) {
        Object.defineProperty(window, 'localStorage', { get() { throw new Error('Storage blocked'); } });
      } else if (options.language) {
        window.localStorage.setItem('ctek-language', options.language);
      }
    },
  });
  pages.push(page);
  await new Promise(resolve => page.window.addEventListener('load', resolve));
  return page.window;
}

afterEach(() => { pages.splice(0).forEach(page => page.window.close()); });

test('de gateway onthoudt taal en vertaalt na wisselen', async () => {
  const window = await openPage(undefined, { language: 'en' });
  expect(window.document.documentElement.lang).toBe('en');
  expect(window.document.getElementById('label').textContent).toBe('Save');
  const selector = window.document.getElementById('ui-language');
  selector.value = 'de';
  selector.dispatchEvent(new window.Event('change'));
  expect(window.document.getElementById('label').textContent).toBe('Speichern');
  expect(window.localStorage.getItem('ctek-language')).toBe('de');
});

test('ongeldige opgeslagen taal valt terug op Nederlands', async () => {
  const window = await openPage(undefined, { language: 'invalid' });
  expect(window.document.documentElement.lang).toBe('nl');
});

test('taal en sensorbediening blijven werken als de browser opslag blokkeert', async () => {
  const window = await openPage(undefined, { storageBlocked: true });
  const selector = window.document.getElementById('ui-language');
  selector.value = 'en';
  selector.dispatchEvent(new window.Event('change'));
  expect(window.document.getElementById('label').textContent).toBe('Save');
  const slots = window.document.querySelectorAll('.sensor-slot');
  expect(slots[1].hidden).toBe(true);
  expect(slots[2].hidden).toBe(true);
  window.document.getElementById('add-sensor').click();
  expect(slots[1].hidden).toBe(false);
  expect(slots[2].hidden).toBe(true);
});

test.each([
  [false, false, false, false],
  [true, false, false, false],
  [true, false, true, false],
  [true, true, true, false],
  [false, true, true, true],
])(
  'plus toont elke lege plek pas na klikken (%j, %j, %j, %j)', async (...configured) => {
    const window = await openPage(configured);
    const empty = [...window.document.querySelectorAll('[data-configured="0"]')];
    const add = window.document.getElementById('add-sensor');
    expect(empty.filter(slot => !slot.hidden)).toHaveLength(0);
    expect([...window.document.querySelectorAll('[data-configured="1"]')].every(slot => !slot.hidden)).toBe(true);
    for (let count = 1; count <= empty.length; count++) {
      expect(add.hidden).toBe(false);
      add.click();
      expect(empty.filter(slot => !slot.hidden)).toHaveLength(count);
    }
    expect(add.hidden).toBe(true);
    expect([...window.document.querySelectorAll('[data-configured="1"]')].every(slot => !slot.hidden)).toBe(true);
  },
);

test('vier ingestelde sensoren verbergen de plusknop', async () => {
  const window = await openPage([true, true, true, true]);
  expect(window.document.getElementById('add-sensor').hidden).toBe(true);
});
