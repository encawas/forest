'use strict';

const express = require('express');
const { EchoForestStateEngine } = require('./stateEngine');

function createApp(options = {}) {
  const app = express();
  const engine = options.engine || new EchoForestStateEngine(options.engineOptions);

  app.use(express.json());

  app.get('/health', (req, res) => {
    res.json({ ok: true, service: 'echoforest-backend' });
  });

  app.get('/api/state', (req, res) => {
    res.json(engine.getState());
  });

  app.get('/api/logs', (req, res) => {
    res.json({ logs: engine.getLogs() });
  });

  app.post('/api/control/start_focus', (req, res, next) => {
    try {
      const body = req.body || {};
      const state = engine.startFocus({
        goal: body.goal,
        source: body.source || 'web',
      });
      res.json({ ok: true, state });
    } catch (error) {
      next(error);
    }
  });

  app.post('/api/control/start_break', (req, res, next) => {
    try {
      const body = req.body || {};
      const state = engine.startBreak({
        source: body.source || 'web',
      });
      res.json({ ok: true, state });
    } catch (error) {
      next(error);
    }
  });

  app.post('/api/control/end_focus', (req, res, next) => {
    try {
      const body = req.body || {};
      const result = engine.endFocus({
        source: body.source || 'web',
      });
      res.json({ ok: true, state: result.state, log: result.log });
    } catch (error) {
      next(error);
    }
  });

  app.post('/api/device_data', (req, res, next) => {
    try {
      const body = req.body || {};
      const state = engine.ingestEnvironment({
        light: body.light,
        noise: body.noise,
        temp: body.temp,
        humi: body.humi,
        source: body.source || 's3',
        deviceId: body.deviceId || 's3-main',
      });
      res.json({ ok: true, state });
    } catch (error) {
      next(error);
    }
  });

  app.post('/api/devices/heartbeat', (req, res, next) => {
    try {
      const body = req.body || {};
      const state = engine.heartbeat({
        deviceId: body.deviceId,
        role: body.role,
        source: body.source,
      });
      res.json({ ok: true, state });
    } catch (error) {
      next(error);
    }
  });

  app.use((error, req, res, next) => {
    const status = error instanceof TypeError || error instanceof SyntaxError ? 400 : 500;
    res.status(status).json({
      ok: false,
      error: error.message || 'internal server error',
    });
  });

  return { app, engine };
}

module.exports = { createApp };
