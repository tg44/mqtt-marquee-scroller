<script lang="ts">
  import { api, type MqttConfig } from '../lib/api';
  import { onMount } from 'svelte';

  let mqtt: MqttConfig | null = $state(null);
  let message = $state('');
  let loading = $state(true);

  onMount(async () => {
    try {
      mqtt = await api.getMqtt();
    } catch {
      message = 'Failed to load MQTT settings';
    }
    loading = false;
  });

  async function save() {
    if (!mqtt) return;
    message = '';
    try {
      const res = await api.saveMqtt(mqtt);
      if (res.restarting) {
        message = 'Saved! Device restarting...';
      } else {
        message = 'Saved!';
      }
    } catch {
      message = 'Failed to save';
    }
  }
</script>

{#if loading}
  <p>Loading...</p>
{:else if mqtt}
  <div class="card">
    <h2>MQTT Configuration</h2>

    <div class="checkbox-row">
      <input type="checkbox" bind:checked={mqtt.enabled} />
      <span>Enable MQTT</span>
    </div>

    <label>Device Name</label>
    <input type="text" maxlength="60" bind:value={mqtt.deviceId} />

    <label>Server URL (no http://)</label>
    <input type="text" maxlength="60" bind:value={mqtt.url} />

    <label>Port</label>
    <input type="number" bind:value={mqtt.port} />

    <label>Topic</label>
    <input type="text" bind:value={mqtt.topic} />

    <label>Face Topic</label>
    <input type="text" bind:value={mqtt.faceTopic} />
  </div>

  <button class="save" onclick={save}>Save & Restart</button>

  {#if message}
    <div class="toast">{message}</div>
  {/if}
{/if}
