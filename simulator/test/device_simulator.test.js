'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const { createApp } = require('../../backend/src/app');
const { runScenario } = require('../device_simulator');

function listen(app) {
  return new Promise((resolve) => {
    const server = app.listen(0, '127.0.0.1', () => {
      const address = server.address();
      resolve({ server, baseUrl: `http://127.0.0.1:${address.port}` });
    });
  });
}

test('device simulator drives the backend through a full demo scenario', async (t) => {
  const { app } = createApp();
  const { server, baseUrl } = await listen(app);
  t.after(() => server.close());

  const timeline = await runScenario({
    baseUrl,
    goal: 'simulator test session',
    scenario: [
      { light: 320, noise: 38, temp: 24, humi: 50, label: 'normal' },
      { light: 80, noise: 40, temp: 24, humi: 50, label: 'dark' },
      { light: 260, noise: 78, temp: 24, humi: 50, label: 'noisy' },
    ],
  });

  assert.equal(timeline[0].step, 'start_focus');
  assert.equal(timeline.at(-1).step, 'end_focus');
  assert.equal(timeline.at(-1).forest_state, 'completed');
  assert.ok(timeline.at(-1).log);
  assert.equal(timeline.at(-1).log.goal, 'simulator test session');
  assert.ok(timeline.some((item) => item.forest_state === 'light_warning'));
  assert.ok(timeline.some((item) => item.forest_state === 'noise_warning'));
});
