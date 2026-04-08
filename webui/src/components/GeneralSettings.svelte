<script lang="ts">
  import { api, type Settings } from '../lib/api';
  import { onMount } from 'svelte';

  let settings: Settings | null = $state(null);
  let message = $state('');
  let loading = $state(true);

  onMount(async () => {
    try {
      settings = await api.getSettings();
    } catch {
      message = 'Failed to load settings';
    }
    loading = false;
  });

  async function save() {
    if (!settings) return;
    message = '';
    try {
      await api.saveSettings(settings);
      message = 'Settings saved!';
      setTimeout(() => (message = ''), 3000);
    } catch {
      message = 'Failed to save settings';
    }
  }
</script>

{#if loading}
  <p>Loading...</p>
{:else if settings}
  <div class="card">
    <h2>Time & Display</h2>

    <label>UTC Offset (hours)</label>
    <input type="number" min="-12" max="14" bind:value={settings.utcOffsetHours} />

    <div class="checkbox-row">
      <input type="checkbox" bind:checked={settings.is24Hour} />
      <span>24-hour clock</span>
    </div>

    <div class="checkbox-row">
      <input type="checkbox" bind:checked={settings.isPM} />
      <span>Show PM indicator (12h mode)</span>
    </div>

    <div class="checkbox-row">
      <input type="checkbox" bind:checked={settings.flashSeconds} />
      <span>Flash : separator</span>
    </div>

    <label>Display On</label>
    <input type="time" bind:value={settings.startTime} />

    <label>Display Off</label>
    <input type="time" bind:value={settings.endTime} />

    <label>Brightness (0-15)</label>
    <input type="number" min="0" max="15" bind:value={settings.brightness} />

    <label>Scroll Speed</label>
    <select bind:value={settings.scrollSpeed}>
      <option value={35}>Slow</option>
      <option value={25}>Normal</option>
      <option value={15}>Fast</option>
      <option value={10}>Very Fast</option>
    </select>

    <label>Minutes Between Scrolling (1-10)</label>
    <input type="number" min="1" max="10" bind:value={settings.refreshMinutes} />
  </div>

  <div class="card">
    <h2>Security</h2>

    <div class="checkbox-row">
      <input type="checkbox" bind:checked={settings.basicAuth} />
      <span>Require authentication</span>
    </div>

    <label>Username</label>
    <input type="text" maxlength="20" bind:value={settings.username} />

    <label>Password</label>
    <input type="password" bind:value={settings.password} />
  </div>

  <button class="save" onclick={save}>Save Settings</button>

  {#if message}
    <div class="toast">{message}</div>
  {/if}
{/if}
