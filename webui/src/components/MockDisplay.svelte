<script lang="ts">
  import { api } from '../lib/api';
  import { onMount } from 'svelte';

  interface Props {
    panelCount: number;
    displayOn: boolean;
    onPanelClick?: (panelIndex: number) => void;
  }

  let { panelCount, displayOn, onPanelClick }: Props = $props();

  // 8 rows x (panelCount * 8) cols
  let pixels: number[][] = $state([]);
  let loading = $state(true);

  export async function refreshPixels() {
    try {
      const data = await api.getDisplayPixels();
      pixels = data.pixels;
    } catch {
      // keep last known state
    }
    loading = false;
  }

  // Auto-refresh every 2s
  let interval: ReturnType<typeof setInterval>;

  onMount(() => {
    refreshPixels();
    interval = setInterval(refreshPixels, 2000);
    return () => clearInterval(interval);
  });

  // Split pixels into panels for click handling
  function getPanelPixels(panelIdx: number): boolean[][] {
    if (!pixels.length) {
      return Array.from({ length: 8 }, () => Array(8).fill(false));
    }
    return pixels.map(row => {
      const start = panelIdx * 8;
      return row.slice(start, start + 8).map(v => v === 1);
    });
  }
</script>

<div class="mock-display">
  {#each Array(panelCount) as _, pi}
    {@const panel = getPanelPixels(pi)}
    <button class="panel" onclick={() => onPanelClick?.(pi)} title="Click to edit panel {pi + 1}">
      {#each panel as row}
        {#each row as pixel}
          <div class="pixel" class:on={pixel && displayOn}></div>
        {/each}
      {/each}
    </button>
  {/each}
  <button class="refresh-btn" onclick={refreshPixels} title="Refresh display">&#x21bb;</button>
</div>

<style>
  .mock-display {
    display: flex;
    gap: 3px;
    background: #111;
    padding: 8px;
    border-radius: 6px;
    justify-content: center;
    align-items: center;
    overflow-x: auto;
    position: relative;
  }
  .panel {
    display: grid;
    grid-template-columns: repeat(8, 1fr);
    grid-template-rows: repeat(8, 1fr);
    gap: 1px;
    min-width: 40px;
    background: transparent;
    border: 1px solid transparent;
    padding: 0;
    cursor: pointer;
    border-radius: 2px;
  }
  .panel:hover {
    border-color: #333;
  }
  .pixel {
    width: 4px;
    height: 4px;
    border-radius: 50%;
    background: #1a1a1a;
  }
  .pixel.on {
    background: #00ff41;
    box-shadow: 0 0 3px #00ff41;
  }
  .refresh-btn {
    background: transparent;
    border: none;
    color: #666;
    font-size: 1.2rem;
    cursor: pointer;
    padding: 0.25rem;
    line-height: 1;
    margin-left: 4px;
  }
  .refresh-btn:hover {
    color: #aaa;
  }
</style>
