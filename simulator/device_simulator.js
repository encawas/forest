#!/usr/bin/env node
'use strict';

const DEFAULT_BASE_URL = process.env.ECHOFOREST_BACKEND_URL || 'http://127.0.0.1:3001';

const DEFAULT_SCENARIO = [
  { light: 320, noise: 38, temp: 24, humi: 50, label: 'normal' },
  { light: 80, noise: 40, temp: 24, humi: 50, label: 'light_warning' },
  { light: 260, noise: 78, temp: 24, humi: 52, label: 'noise_warning' },
  { light: 280, noise: 42, temp: 31, humi: 74, label: 'comfort_warning' },
  { light: 300, noise: 41, temp: 24.5, humi: 50, label: 'normal_recovered' },
];

async function requestJson(baseUrl, path, options = {}) {
  const response = await fetch(`${baseUrl}${path}`, {
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) },
    ...options,
  });
  const payload = await response.json();
  if (!response.ok || payload.ok === false) {
    throw new Error(payload.error || `Request failed: ${path}`);
  }
  return payload;
}

async function postControl(baseUrl, path, source, extra = {}) {
  return requestJson(baseUrl, path, {
    method: 'POST',
    body: JSON.stringify({ source, ...extra }),
  });
}

async function postEnvironment(baseUrl, sample, deviceId = 's3-main') {
  return requestJson(baseUrl, '/api/device_data', {
    method: 'POST',
    body: JSON.stringify({
      deviceId,
      source: 's3',
      light: sample.light,
      noise: sample.noise,
      temp: sample.temp,
      humi: sample.humi,
    }),
  });
}

async function postHeartbeat(baseUrl, deviceId, role) {
  return requestJson(baseUrl, '/api/devices/heartbeat', {
    method: 'POST',
    body: JSON.stringify({ deviceId, role, source: role }),
  });
}

async function runScenario({ baseUrl = DEFAULT_BASE_URL, goal = 'EchoForest simulator session', scenario = DEFAULT_SCENARIO } = {}) {
  const timeline = [];

  await postHeartbeat(baseUrl, 'c5-control', 'c5');
  await postHeartbeat(baseUrl, 's3-main', 's3');

  let result = await postControl(baseUrl, '/api/control/start_focus', 'web', { goal });
  timeline.push({ step: 'start_focus', mode: result.state.mode, forest_state: result.state.forest_state });

  for (const sample of scenario) {
    result = await postEnvironment(baseUrl, sample);
    timeline.push({
      step: sample.label,
      light: sample.light,
      noise: sample.noise,
      temp: sample.temp,
      humi: sample.humi,
      mode: result.state.mode,
      forest_state: result.state.forest_state,
      reason: result.state.reason,
    });
  }

  result = await postControl(baseUrl, '/api/control/start_break', 'c5');
  timeline.push({ step: 'start_break', mode: result.state.mode, forest_state: result.state.forest_state });

  result = await postControl(baseUrl, '/api/control/start_focus', 'c5');
  timeline.push({ step: 'resume_focus', mode: result.state.mode, forest_state: result.state.forest_state });

  result = await postControl(baseUrl, '/api/control/end_focus', 'web');
  timeline.push({ step: 'end_focus', mode: result.state.mode, forest_state: result.state.forest_state, log: result.log });

  return timeline;
}

async function main() {
  const baseUrl = process.argv[2] || DEFAULT_BASE_URL;
  const timeline = await runScenario({ baseUrl });
  console.log(JSON.stringify({ ok: true, baseUrl, timeline }, null, 2));
}

if (require.main === module) {
  main().catch((error) => {
    console.error(JSON.stringify({ ok: false, error: error.message }, null, 2));
    process.exitCode = 1;
  });
}

module.exports = {
  DEFAULT_SCENARIO,
  runScenario,
  requestJson,
};
