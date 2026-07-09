'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const {
  EchoForestStateEngine,
  FOREST_STATE,
  MODE,
} = require('../src/stateEngine');

test('complete EchoForest loop: start focus, ingest S3 data, warn, finish, create log', () => {
  const engine = new EchoForestStateEngine({
    thresholds: {
      minLight: 120,
      maxNoise: 65,
      minTemp: 18,
      maxTemp: 28,
      minHumi: 30,
      maxHumi: 70,
    },
  });

  let state = engine.startFocus({
    goal: 'finish probability review',
    source: 'web',
    timestamp: '2026-07-09T08:00:00.000Z',
  });

  assert.equal(state.mode, MODE.FOCUS);
  assert.equal(state.forest_state, FOREST_STATE.GROWING);
  assert.equal(state.session.goal, 'finish probability review');

  state = engine.ingestEnvironment({
    light: 300,
    noise: 40,
    temp: 24,
    humi: 50,
    source: 's3',
    deviceId: 's3-main',
    timestamp: '2026-07-09T08:01:00.000Z',
  });

  assert.equal(state.forest_state, FOREST_STATE.GROWING);
  assert.equal(state.environment.light, 300);
  assert.equal(state.devices['s3-main'].online, true);

  state = engine.ingestEnvironment({
    light: 80,
    noise: 40,
    temp: 24,
    humi: 50,
    source: 's3',
    deviceId: 's3-main',
    timestamp: '2026-07-09T08:02:00.000Z',
  });

  assert.equal(state.forest_state, FOREST_STATE.LIGHT_WARNING);
  assert.equal(state.session.warnings.light_warning, 1);

  const result = engine.endFocus({
    source: 'c5',
    timestamp: '2026-07-09T08:30:00.000Z',
  });

  assert.equal(result.state.mode, MODE.COMPLETED);
  assert.equal(result.state.forest_state, FOREST_STATE.COMPLETED);
  assert.ok(result.log);
  assert.equal(result.log.goal, 'finish probability review');
  assert.equal(result.log.durationSec, 1800);
  assert.equal(result.log.environmentSummary.samples, 2);
  assert.equal(result.log.warnings.light_warning, 1);
  assert.equal(engine.getLogs().length, 1);
});

test('state priority: break mode overrides environment warnings', () => {
  const engine = new EchoForestStateEngine();
  engine.startFocus({ timestamp: '2026-07-09T08:00:00.000Z' });
  engine.startBreak({ source: 'c5', timestamp: '2026-07-09T08:05:00.000Z' });

  const state = engine.ingestEnvironment({
    light: 1,
    noise: 100,
    temp: 35,
    humi: 90,
    timestamp: '2026-07-09T08:06:00.000Z',
  });

  assert.equal(state.mode, MODE.BREAK);
  assert.equal(state.forest_state, FOREST_STATE.BREAK);
});

test('invalid environment values are rejected', () => {
  const engine = new EchoForestStateEngine();
  assert.throws(() => {
    engine.ingestEnvironment({
      light: 'bad',
      noise: 40,
      temp: 24,
      humi: 50,
    });
  }, /light must be a finite number/);
});
