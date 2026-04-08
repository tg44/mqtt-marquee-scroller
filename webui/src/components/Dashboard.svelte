<script lang="ts">
  import { api, type Dashboard } from '../lib/api';
  import { onMount } from 'svelte';
  import MockDisplay from './MockDisplay.svelte';
  import FaceEditor from './FaceEditor.svelte';

  let data: Dashboard | null = $state(null);
  let error = $state('');
  let editingPanel: number | null = $state(null);
  let interval: ReturnType<typeof setInterval>;

  function formatUptime(seconds: number): string {
    const d = Math.floor(seconds / 86400);
    const h = Math.floor((seconds % 86400) / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    if (d > 0) return `${d}d ${h}h ${m}m`;
    if (h > 0) return `${h}h ${m}m`;
    return `${m}m`;
  }

  function formatKB(bytes: number): string {
    return (bytes / 1024).toFixed(1);
  }

  async function refresh() {
    try {
      data = await api.getDashboard();
      error = '';
    } catch {
      error = 'Connection lost';
    }
  }

  async function toggleDisplay() {
    try {
      const res = await api.toggleDisplay();
      if (data) data.display.on = res.displayOn;
    } catch { /* silent */ }
  }

  async function onBrightnessChange(e: Event) {
    const val = parseInt((e.target as HTMLInputElement).value);
    if (data) data.display.brightness = val;
    try {
      await api.setBrightness(val);
    } catch { /* silent */ }
  }

  onMount(() => {
    refresh();
    interval = setInterval(refresh, 2000);
    return () => clearInterval(interval);
  });
</script>

{#if error && !data}
  <div class="card"><p>{error}</p></div>
{:else if data}
  <MockDisplay
    panelCount={data.display.panels}
    displayOn={data.display.on}
    onPanelClick={(i) => (editingPanel = i)}
  />

  {#if editingPanel !== null}
    <FaceEditor
      panelIndex={editingPanel}
      faces={data.mqtt.panelFaces}
      panelCount={data.display.panels}
      onClose={() => (editingPanel = null)}
      onApply={() => { editingPanel = null; refresh(); }}
    />
  {/if}

  <div class="status-grid">
    <div class="card">
      <h3>WiFi</h3>
      <div class="stat"><span>SSID</span><span>{data.wifi.ssid}</span></div>
      <div class="stat"><span>IP</span><span>{data.wifi.ip}</span></div>
      <div class="stat"><span>Signal</span><span>{data.wifi.signal}%</span></div>
    </div>

    <div class="card">
      <h3>Device</h3>
      <div class="stat"><span>Uptime</span><span>{formatUptime(data.device.uptimeSeconds)}</span></div>
      <div class="stat"><span>Free RAM</span><span>{formatKB(data.device.freeHeap)} / 80 KB</span></div>
      <div class="stat"><span>CPU</span><span>{data.device.cpuMhz} MHz</span></div>
      <div class="stat"><span>Reset</span><span>{data.device.resetReason}</span></div>
    </div>

    <div class="card">
      <div class="card-header">
        <h3>Display</h3>
        <button class="icon-btn" onclick={toggleDisplay} title="Toggle display">
          {data.display.on ? '\u{1F7E2}' : '\u{1F534}'}
        </button>
      </div>
      <div class="stat"><span>Status</span><span>{data.display.on ? 'ON' : 'OFF'}</span></div>
      <div class="stat"><span>Panels</span><span>{data.display.panels}</span></div>
      <div class="stat"><span>Brightness</span><span>{data.display.brightness}/15</span></div>
      <input
        type="range"
        min="0"
        max="15"
        value={data.display.brightness}
        onchange={onBrightnessChange}
        class="slider"
      />
    </div>

    <div class="card">
      <h3>MQTT</h3>
      {#if data.mqtt.enabled}
        <div class="stat">
          <span>Status</span>
          <span class:connected={data.mqtt.connected} class:disconnected={!data.mqtt.connected}>
            {data.mqtt.connected ? 'Connected' : 'Disconnected'}
          </span>
        </div>
        <div class="stat"><span>Server</span><span>{data.mqtt.url}:{data.mqtt.port}</span></div>
        <div class="stat-full"><span>Topic</span><span>{data.mqtt.topic}</span></div>
        <div class="stat-full"><span>Face</span><span>{data.mqtt.faceTopic}</span></div>
      {:else}
        <div class="stat"><span>Status</span><span>Disabled</span></div>
      {/if}
    </div>
  </div>

  <div class="card">
    <h3>Scroll Buffer</h3>
    <p class="message">{data.mqtt.scrollBuffer || 'Empty'}</p>
  </div>

  <div class="card">
    <h3>Last Message</h3>
    <p class="message">{data.mqtt.lastMessage || 'No messages yet'}</p>
  </div>

  {#if error}
    <div class="toast" style="background: var(--accent)">{error}</div>
  {/if}
{:else}
  <p>Loading...</p>
{/if}

<style>
  .status-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 0.75rem;
    margin-top: 0.75rem;
  }

  h3 {
    font-size: 0.85rem;
    color: var(--text-muted);
    margin-bottom: 0.5rem;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }

  .card-header h3 {
    margin-bottom: 0;
  }

  .icon-btn {
    background: transparent;
    border: none;
    font-size: 1.1rem;
    cursor: pointer;
    padding: 0.2rem;
    line-height: 1;
  }

  .stat {
    display: flex;
    justify-content: space-between;
    font-size: 0.85rem;
    padding: 0.15rem 0;
  }

  .stat span:first-child {
    color: var(--text-muted);
  }

  .stat-full {
    font-size: 0.85rem;
    padding: 0.15rem 0;
  }

  .stat-full span:first-child {
    color: var(--text-muted);
    display: block;
  }

  .stat-full span:last-child {
    word-break: break-all;
    font-size: 0.8rem;
  }

  .connected { color: var(--success); }
  .disconnected { color: var(--accent); }

  .message {
    font-family: monospace;
    word-break: break-all;
    color: var(--text);
    font-size: 0.9rem;
  }

  .slider {
    width: 100%;
    margin-top: 0.25rem;
    accent-color: var(--success);
  }

  @media (max-width: 400px) {
    .status-grid {
      grid-template-columns: 1fr;
    }
  }
</style>
