'use strict';

const DEFAULT_THRESHOLDS = Object.freeze({
  minLight: 120,
  maxNoise: 65,
  minTemp: 18,
  maxTemp: 28,
  minHumi: 30,
  maxHumi: 70,
});

const MODE = Object.freeze({
  IDLE: 'idle',
  FOCUS: 'focus',
  BREAK: 'break',
  COMPLETED: 'completed',
});

const FOREST_STATE = Object.freeze({
  IDLE: 'idle',
  GROWING: 'growing',
  LIGHT_WARNING: 'light_warning',
  NOISE_WARNING: 'noise_warning',
  COMFORT_WARNING: 'comfort_warning',
  BREAK: 'break',
  COMPLETED: 'completed',
});

function isoNow() {
  return new Date().toISOString();
}

function clone(value) {
  return JSON.parse(JSON.stringify(value));
}

function toFiniteNumber(value, fieldName) {
  const numberValue = Number(value);
  if (!Number.isFinite(numberValue)) {
    throw new TypeError(`${fieldName} must be a finite number`);
  }
  return numberValue;
}

function createEmptyEnvironment() {
  return {
    light: null,
    noise: null,
    temp: null,
    humi: null,
    updatedAt: null,
    source: null,
  };
}

function createEnvironmentSummary() {
  return {
    samples: 0,
    light: { min: null, max: null, avg: null },
    noise: { min: null, max: null, avg: null },
    temp: { min: null, max: null, avg: null },
    humi: { min: null, max: null, avg: null },
  };
}

function updateRange(summary, field, value) {
  const current = summary[field];
  current.min = current.min === null ? value : Math.min(current.min, value);
  current.max = current.max === null ? value : Math.max(current.max, value);
  current.avg = current.avg === null
    ? value
    : Number(((current.avg * (summary.samples - 1) + value) / summary.samples).toFixed(2));
}

function summarizeReason(forestState) {
  switch (forestState) {
    case FOREST_STATE.GROWING:
      return '环境正常，森林成长中';
    case FOREST_STATE.LIGHT_WARNING:
      return '光线过暗，建议提高照明';
    case FOREST_STATE.NOISE_WARNING:
      return '噪声偏高，建议降低环境噪声';
    case FOREST_STATE.COMFORT_WARNING:
      return '温湿度不适，建议调整学习环境';
    case FOREST_STATE.BREAK:
      return '休息模式中';
    case FOREST_STATE.COMPLETED:
      return '学习完成';
    default:
      return '等待学习开始';
  }
}

function evaluateForestState({ mode, environment, thresholds = DEFAULT_THRESHOLDS }) {
  if (mode === MODE.BREAK) {
    return { forestState: FOREST_STATE.BREAK, reason: summarizeReason(FOREST_STATE.BREAK) };
  }

  if (mode === MODE.COMPLETED) {
    return { forestState: FOREST_STATE.COMPLETED, reason: summarizeReason(FOREST_STATE.COMPLETED) };
  }

  if (mode !== MODE.FOCUS) {
    return { forestState: FOREST_STATE.IDLE, reason: summarizeReason(FOREST_STATE.IDLE) };
  }

  if (environment.light !== null && environment.light < thresholds.minLight) {
    return { forestState: FOREST_STATE.LIGHT_WARNING, reason: summarizeReason(FOREST_STATE.LIGHT_WARNING) };
  }

  if (environment.noise !== null && environment.noise > thresholds.maxNoise) {
    return { forestState: FOREST_STATE.NOISE_WARNING, reason: summarizeReason(FOREST_STATE.NOISE_WARNING) };
  }

  const tempBad = environment.temp !== null
    && (environment.temp < thresholds.minTemp || environment.temp > thresholds.maxTemp);
  const humiBad = environment.humi !== null
    && (environment.humi < thresholds.minHumi || environment.humi > thresholds.maxHumi);

  if (tempBad || humiBad) {
    return { forestState: FOREST_STATE.COMFORT_WARNING, reason: summarizeReason(FOREST_STATE.COMFORT_WARNING) };
  }

  return { forestState: FOREST_STATE.GROWING, reason: summarizeReason(FOREST_STATE.GROWING) };
}

class EchoForestStateEngine {
  constructor(options = {}) {
    this.thresholds = { ...DEFAULT_THRESHOLDS, ...(options.thresholds || {}) };
    this.state = {
      version: 0,
      mode: MODE.IDLE,
      forest_state: FOREST_STATE.IDLE,
      reason: summarizeReason(FOREST_STATE.IDLE),
      environment: createEmptyEnvironment(),
      session: null,
      devices: {},
      logs: [],
      updatedAt: isoNow(),
    };
  }

  getState() {
    return clone(this.state);
  }

  startFocus({ goal = '', source = 'unknown', timestamp = isoNow() } = {}) {
    if (this.state.session && this.state.session.active) {
      this.state.mode = MODE.FOCUS;
      if (goal) this.state.session.goal = String(goal);
      this._recordEvent('focus_resumed', source, timestamp);
    } else {
      this.state.session = {
        id: `session-${Date.parse(timestamp) || Date.now()}`,
        goal: String(goal || '').trim(),
        active: true,
        startedAt: timestamp,
        endedAt: null,
        events: [],
        warnings: {
          light_warning: 0,
          noise_warning: 0,
          comfort_warning: 0,
        },
        environmentSummary: createEnvironmentSummary(),
      };
      this.state.mode = MODE.FOCUS;
      this._recordEvent('focus_started', source, timestamp);
    }

    this._recompute(timestamp);
    return this.getState();
  }

  startBreak({ source = 'unknown', timestamp = isoNow() } = {}) {
    if (!this.state.session || !this.state.session.active) {
      return this.startFocus({ goal: '', source, timestamp });
    }

    this.state.mode = MODE.BREAK;
    this._recordEvent('break_started', source, timestamp);
    this._recompute(timestamp);
    return this.getState();
  }

  endFocus({ source = 'unknown', timestamp = isoNow() } = {}) {
    if (!this.state.session || !this.state.session.active) {
      this.state.mode = MODE.COMPLETED;
      this._recompute(timestamp);
      return { state: this.getState(), log: null };
    }

    this.state.session.active = false;
    this.state.session.endedAt = timestamp;
    this.state.mode = MODE.COMPLETED;
    this._recordEvent('focus_completed', source, timestamp);
    this._recompute(timestamp);

    const log = this._buildLog(this.state.session);
    this.state.logs.unshift(log);
    return { state: this.getState(), log: clone(log) };
  }

  ingestEnvironment({ light, noise, temp, humi, source = 's3', deviceId = 's3-main', timestamp = isoNow() } = {}) {
    const nextEnvironment = {
      light: toFiniteNumber(light, 'light'),
      noise: toFiniteNumber(noise, 'noise'),
      temp: toFiniteNumber(temp, 'temp'),
      humi: toFiniteNumber(humi, 'humi'),
      updatedAt: timestamp,
      source,
    };

    this.state.environment = nextEnvironment;
    this.heartbeat({ deviceId, role: 's3', source, timestamp });

    if (this.state.session && this.state.session.active) {
      const summary = this.state.session.environmentSummary;
      summary.samples += 1;
      updateRange(summary, 'light', nextEnvironment.light);
      updateRange(summary, 'noise', nextEnvironment.noise);
      updateRange(summary, 'temp', nextEnvironment.temp);
      updateRange(summary, 'humi', nextEnvironment.humi);
    }

    const previousForestState = this.state.forest_state;
    this._recompute(timestamp);

    if (
      this.state.session
      && this.state.session.active
      && this.state.forest_state !== previousForestState
      && Object.prototype.hasOwnProperty.call(this.state.session.warnings, this.state.forest_state)
    ) {
      this.state.session.warnings[this.state.forest_state] += 1;
      this._recordEvent(this.state.forest_state, source, timestamp);
    }

    return this.getState();
  }

  heartbeat({ deviceId, role = 'unknown', source = 'unknown', timestamp = isoNow() } = {}) {
    const id = String(deviceId || role || source || 'unknown');
    this.state.devices[id] = {
      id,
      role,
      source,
      online: true,
      lastSeenAt: timestamp,
    };
    this._touch(timestamp);
    return this.getState();
  }

  getLogs() {
    return clone(this.state.logs);
  }

  _recordEvent(type, source, timestamp) {
    if (!this.state.session) return;
    this.state.session.events.push({
      type,
      source,
      timestamp,
      mode: this.state.mode,
      forest_state: this.state.forest_state,
    });
  }

  _recompute(timestamp) {
    const result = evaluateForestState({
      mode: this.state.mode,
      environment: this.state.environment,
      thresholds: this.thresholds,
    });
    this.state.forest_state = result.forestState;
    this.state.reason = result.reason;
    this._touch(timestamp);
  }

  _touch(timestamp) {
    this.state.version += 1;
    this.state.updatedAt = timestamp;
  }

  _buildLog(session) {
    const startedAt = Date.parse(session.startedAt);
    const endedAt = Date.parse(session.endedAt);
    const durationSec = Number.isFinite(startedAt) && Number.isFinite(endedAt)
      ? Math.max(0, Math.round((endedAt - startedAt) / 1000))
      : null;

    const warningCount = Object.values(session.warnings).reduce((sum, count) => sum + count, 0);
    const summary = warningCount === 0
      ? '本轮学习环境整体稳定，森林顺利成长。'
      : `本轮学习共出现 ${warningCount} 次环境提醒，建议根据光线、噪声或舒适度提示调整环境。`;

    return {
      id: `log-${session.id}`,
      sessionId: session.id,
      goal: session.goal,
      startedAt: session.startedAt,
      endedAt: session.endedAt,
      durationSec,
      warnings: clone(session.warnings),
      environmentSummary: clone(session.environmentSummary),
      summary,
    };
  }
}

module.exports = {
  EchoForestStateEngine,
  DEFAULT_THRESHOLDS,
  MODE,
  FOREST_STATE,
  evaluateForestState,
};
