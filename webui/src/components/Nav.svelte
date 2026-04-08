<script lang="ts">
  interface Props {
    current: string;
    onNavigate: (view: string) => void;
  }
  let { current, onNavigate }: Props = $props();

  const tabs = [
    { id: 'dashboard', label: 'Dashboard' },
    { id: 'settings', label: 'Settings' },
    { id: 'mqtt', label: 'MQTT' },
    { id: 'actions', label: 'Actions' },
  ];
</script>

<nav>
  {#each tabs as tab}
    <button class:active={current === tab.id} onclick={() => onNavigate(tab.id)}>
      {tab.label}
    </button>
  {/each}
  <a href="/update" target="_blank" rel="noopener">
    <button>Firmware</button>
  </a>
</nav>
