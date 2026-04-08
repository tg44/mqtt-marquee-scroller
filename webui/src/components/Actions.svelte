<script lang="ts">
  import { api } from '../lib/api';

  let message = $state('');

  async function toggleDisplay() {
    try {
      const res = await api.toggleDisplay();
      message = `Display is now ${res.displayOn ? 'ON' : 'OFF'}`;
      setTimeout(() => (message = ''), 3000);
    } catch {
      message = 'Failed';
    }
  }

  async function refreshNtp() {
    try {
      await api.refreshNtp();
      message = 'NTP refreshed!';
      setTimeout(() => (message = ''), 3000);
    } catch {
      message = 'Failed';
    }
  }

  async function resetSettings() {
    if (!confirm('Reset all settings to defaults?')) return;
    try {
      await api.reset();
      message = 'Settings reset! Device restarting...';
    } catch {
      message = 'Failed';
    }
  }

  async function forgetWifi() {
    if (!confirm('Forget WiFi credentials? You will need to reconnect via AP.')) return;
    try {
      await api.forgetWifi();
      message = 'WiFi forgotten! Device restarting...';
    } catch {
      message = 'Failed';
    }
  }
</script>

<div class="card">
  <h2>Actions</h2>
  <div class="actions">
    <button onclick={refreshNtp}>Refresh NTP</button>
    <button onclick={toggleDisplay}>Toggle Display</button>
    <button class="danger" onclick={resetSettings}>Reset Settings</button>
    <button class="danger" onclick={forgetWifi}>Forget WiFi</button>
  </div>
</div>

{#if message}
  <div class="toast">{message}</div>
{/if}
