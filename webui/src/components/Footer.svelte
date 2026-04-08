<script lang="ts">
  import { api, type Status } from '../lib/api';
  import { onMount } from 'svelte';

  let status: Status | null = $state(null);

  onMount(async () => {
    try {
      status = await api.getStatus();
    } catch {
      // silent fail
    }
  });
</script>

<div class="footer">
  {#if status}
    v{status.version} | WiFi: {status.wifiSignal}%
  {:else}
    Loading...
  {/if}
</div>
