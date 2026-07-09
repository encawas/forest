import { useEffect, useMemo, useState } from 'react';

const EMPTY_STATE = {
  version: 0,
  mode: 'idle',
  forest_state: 'idle',
  reason: '等待后端状态',
  environment: { light: null, noise: null, temp: null, humi: null, updatedAt: null },
  session: null,
  devices: {},
  logs: [],
};

const FOREST_LABELS = {
  idle: '等待学习',
  growing: '森林成长中',
  light_warning: '光线提醒',
  noise_warning: '噪声提醒',
  comfort_warning: '舒适度提醒',
  break: '休息中',
  completed: '学习完成',
};

const MODE_LABELS = {
  idle: '待开始',
  focus: '学习中',
  break: '休息中',
  completed: '已完成',
};

async function requestJson(path, options = {}) {
  const response = await fetch(path, {
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) },
    ...options,
  });
  const payload = await response.json();
  if (!response.ok || payload.ok === false) {
    throw new Error(payload.error || `Request failed: ${path}`);
  }
  return payload;
}

function formatNumber(value, digits = 0) {
  if (value === null || value === undefined || Number.isNaN(Number(value))) return '--';
  return Number(value).toFixed(digits);
}

function deviceStatus(state, role) {
  return Object.values(state.devices || {}).find((device) => device.role === role);
}

export default function App() {
  const [goal, setGoal] = useState('完成本轮专注学习');
  const [state, setState] = useState(EMPTY_STATE);
  const [logs, setLogs] = useState([]);
  const [error, setError] = useState('');
  const [busy, setBusy] = useState(false);

  async function refresh() {
    const nextState = await requestJson('/api/state');
    setState(nextState);
    setLogs(nextState.logs || []);
    setError('');
  }

  async function control(path, body = {}) {
    setBusy(true);
    try {
      const result = await requestJson(path, {
        method: 'POST',
        body: JSON.stringify({ source: 'web', ...body }),
      });
      setState(result.state);
      setLogs(result.state.logs || (result.log ? [result.log] : []));
      setError('');
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  useEffect(() => {
    refresh().catch((err) => setError(err.message));
    const timer = setInterval(() => {
      refresh().catch((err) => setError(err.message));
    }, 1500);
    return () => clearInterval(timer);
  }, []);

  const s3 = useMemo(() => deviceStatus(state, 's3'), [state]);
  const c5 = useMemo(() => deviceStatus(state, 'c5'), [state]);
  const env = state.environment || EMPTY_STATE.environment;

  return (
    <main className="page">
      <section className="hero panel">
        <div>
          <p className="eyebrow">EchoForest AIoT Console</p>
          <h1>{FOREST_LABELS[state.forest_state] || state.forest_state}</h1>
          <p className="reason">{state.reason}</p>
        </div>
        <div className="version">v{state.version}</div>
      </section>

      <section className="grid two">
        <div className="panel">
          <h2>学习控制</h2>
          <label className="field">
            <span>本轮学习目标</span>
            <input value={goal} onChange={(event) => setGoal(event.target.value)} />
          </label>
          <div className="actions">
            <button disabled={busy} onClick={() => control('/api/control/start_focus', { goal })}>开始学习</button>
            <button disabled={busy} onClick={() => control('/api/control/start_break')}>进入休息</button>
            <button disabled={busy} className="danger" onClick={() => control('/api/control/end_focus')}>结束学习</button>
          </div>
          {error && <p className="error">{error}</p>}
        </div>

        <div className="panel">
          <h2>当前状态</h2>
          <div className="statusRows">
            <div><span>模式</span><strong>{MODE_LABELS[state.mode] || state.mode}</strong></div>
            <div><span>森林</span><strong>{FOREST_LABELS[state.forest_state] || state.forest_state}</strong></div>
            <div><span>目标</span><strong>{state.session?.goal || '未开始'}</strong></div>
            <div><span>更新</span><strong>{state.updatedAt || '--'}</strong></div>
          </div>
        </div>
      </section>

      <section className="grid four">
        <Metric title="Light" value={formatNumber(env.light)} suffix="" />
        <Metric title="Noise" value={formatNumber(env.noise)} suffix="" />
        <Metric title="Temp" value={formatNumber(env.temp, 1)} suffix="°C" />
        <Metric title="Humi" value={formatNumber(env.humi)} suffix="%" />
      </section>

      <section className="grid two">
        <div className="panel">
          <h2>设备反馈</h2>
          <div className="deviceList">
            <Device name="S3 主屏/环境感知" device={s3} />
            <Device name="C5 副屏/触摸控制" device={c5} />
          </div>
        </div>

        <div className="panel">
          <h2>本轮事件</h2>
          <ul className="events">
            {(state.session?.events || []).slice(-6).map((event, index) => (
              <li key={`${event.timestamp}-${index}`}>
                <strong>{event.type}</strong>
                <span>{event.source}</span>
                <time>{event.timestamp}</time>
              </li>
            ))}
            {!state.session?.events?.length && <li className="muted">暂无事件</li>}
          </ul>
        </div>
      </section>

      <section className="panel">
        <h2>学习日志</h2>
        <div className="logs">
          {logs.map((log) => (
            <article key={log.id} className="logCard">
              <h3>{log.goal || '未命名学习'}</h3>
              <p>{log.summary}</p>
              <small>{log.startedAt} → {log.endedAt}，用时 {log.durationSec ?? '--'} 秒</small>
            </article>
          ))}
          {logs.length === 0 && <p className="muted">结束学习后会在这里生成本轮日志。</p>}
        </div>
      </section>
    </main>
  );
}

function Metric({ title, value, suffix }) {
  return (
    <div className="metric panel">
      <span>{title}</span>
      <strong>{value}{suffix}</strong>
    </div>
  );
}

function Device({ name, device }) {
  return (
    <div className="device">
      <div>
        <strong>{name}</strong>
        <span>{device?.lastSeenAt || '尚未连接'}</span>
      </div>
      <b className={device?.online ? 'online' : 'offline'}>{device?.online ? '在线' : '离线'}</b>
    </div>
  );
}
