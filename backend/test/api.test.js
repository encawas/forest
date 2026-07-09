'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const { createApp } = require('../src/app');

function listen(app) {
  return new Promise((resolve) => {
    const server = app.listen(0, '127.0.0.1', () => {
      const address = server.address();
      resolve({ server, baseUrl: `http://127.0.0.1:${address.port}` });
    });
  });
}

async function jsonFetch(baseUrl, path, options = {}) {
  const response = await fetch(`${baseUrl}${path}`, {
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) },
    ...options,
  });
  const payload = await response.json();
  return { response, payload };
}

test('HTTP API exposes the full EchoForest loop', async (t) => {
  const { app } = createApp();
  const { server, baseUrl } = await listen(app);
  t.after(() => server.close());

  let result = await jsonFetch(baseUrl, '/api/control/start_focus', {
    method: 'POST',
    body: JSON.stringify({ source: 'web', goal: 'API integration review' }),
  });
  assert.equal(result.response.status, 200);
  assert.equal(result.payload.ok, true);
  assert.equal(result.payload.state.mode, 'focus');
  assert.equal(result.payload.state.session.goal, 'API integration review');

  result = await jsonFetch(baseUrl, '/api/device_data', {
    method: 'POST',
    body: JSON.stringify({
      source: 's3',
      deviceId: 's3-main',
      light: 60,
      noise: 40,
      temp: 24,
      humi: 50,
    }),
  });
  assert.equal(result.response.status, 200);
  assert.equal(result.payload.state.forest_state, 'light_warning');
  assert.equal(result.payload.state.devices['s3-main'].online, true);

  result = await jsonFetch(baseUrl, '/api/devices/heartbeat', {
    method: 'POST',
    body: JSON.stringify({ deviceId: 'c5-control', role: 'c5', source: 'c5' }),
  });
  assert.equal(result.response.status, 200);
  assert.equal(result.payload.state.devices['c5-control'].role, 'c5');

  result = await jsonFetch(baseUrl, '/api/state');
  assert.equal(result.response.status, 200);
  assert.equal(result.payload.forest_state, 'light_warning');

  result = await jsonFetch(baseUrl, '/api/control/end_focus', {
    method: 'POST',
    body: JSON.stringify({ source: 'c5' }),
  });
  assert.equal(result.response.status, 200);
  assert.equal(result.payload.ok, true);
  assert.equal(result.payload.state.mode, 'completed');
  assert.equal(result.payload.log.goal, 'API integration review');

  result = await jsonFetch(baseUrl, '/api/logs');
  assert.equal(result.response.status, 200);
  assert.equal(result.payload.logs.length, 1);
});

test('HTTP API rejects invalid S3 environment payloads', async (t) => {
  const { app } = createApp();
  const { server, baseUrl } = await listen(app);
  t.after(() => server.close());

  const result = await jsonFetch(baseUrl, '/api/device_data', {
    method: 'POST',
    body: JSON.stringify({ light: 'bad', noise: 40, temp: 24, humi: 50 }),
  });

  assert.equal(result.response.status, 400);
  assert.equal(result.payload.ok, false);
  assert.match(result.payload.error, /light must be a finite number/);
});
