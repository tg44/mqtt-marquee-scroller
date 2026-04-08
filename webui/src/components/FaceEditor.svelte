<script lang="ts">
  import { api, type PanelFace } from '../lib/api';

  interface Props {
    panelIndex: number;
    faces: PanelFace[];
    panelCount: number;
    onClose: () => void;
    onApply: () => void;
  }

  let { panelIndex, faces, panelCount, onClose, onApply }: Props = $props();

  let faceType = $state(faces[panelIndex]?.type ?? 0);
  let percent = $state(faces[panelIndex]?.percent ?? 0);
  let character = $state(faces[panelIndex]?.char?.trim() || '');

  // Raw pixel grid (8x8 per panel, expandable)
  let rawSpan = $state(1);
  let rawGrid: boolean[][] = $state(
    Array.from({ length: 8 }, () => Array(8).fill(false))
  );
  let rawLoaded = $state(false);

  // Load current hardware pixels when switching to raw mode
  async function loadHardwarePixels() {
    if (rawLoaded) return;
    try {
      const data = await api.getDisplayPixels();
      const startCol = panelIndex * 8;
      const cols = rawSpan * 8;
      rawGrid = data.pixels.map(row =>
        row.slice(startCol, startCol + cols).map(v => v === 1)
      );
      rawLoaded = true;
    } catch { /* keep empty grid */ }
  }

  $effect(() => {
    if (faceType === 5) loadHardwarePixels();
  });

  const faceTypes = [
    { value: 0, label: 'Empty' },
    { value: 1, label: 'Clock (4-6 panels)' },
    { value: 2, label: 'Binary Clock (1-2 panels)' },
    { value: 3, label: 'Percentage (1 panel)' },
    { value: 4, label: 'Character (1 panel)' },
    { value: 5, label: 'Raw Pixels (drawable)' },
  ];

  function panelsNeeded(type: number): number {
    if (type === 1) return 4;
    if (type === 2) return 2;
    return 1;
  }

  function togglePixel(row: number, col: number) {
    rawGrid[row][col] = !rawGrid[row][col];
    rawGrid = [...rawGrid]; // trigger reactivity
  }

  function resizeGrid() {
    const totalCols = rawSpan * 8;
    rawGrid = Array.from({ length: 8 }, (_, r) =>
      Array.from({ length: totalCols }, (_, c) =>
        rawGrid[r]?.[c] ?? false
      )
    );
  }

  async function apply() {
    if (faceType === 5) {
      // Send raw pixel data
      const pixels = rawGrid.map(row => row.map(p => p ? 1 : 0));
      try {
        await api.postRaw(panelIndex, pixels);
        onApply();
      } catch { /* silent */ }
      return;
    }

    const updated = faces.map((f, i) => ({
      t: f.type,
      p: f.percent,
      c: f.char,
    }));

    updated[panelIndex] = {
      t: faceType,
      p: faceType === 3 ? percent : 0,
      c: faceType === 4 ? (character || ' ') : ' ',
    };

    const needed = panelsNeeded(faceType);
    for (let i = 1; i < needed && (panelIndex + i) < panelCount; i++) {
      updated[panelIndex + i] = { t: 0, p: 0, c: ' ' };
    }

    try {
      await api.setFaces(updated);
      onApply();
    } catch { /* silent */ }
  }

  function handleCharInput(e: Event) {
    const input = e.target as HTMLInputElement;
    character = input.value.slice(-1);
    input.value = character;
  }
</script>

<div class="overlay" onclick={onClose} role="presentation">
  <div class="editor" class:wide={faceType === 5} onclick={(e) => e.stopPropagation()} role="dialog">
    <h3>Panel {panelIndex + 1}</h3>

    <label>Face Type</label>
    <select bind:value={faceType}>
      {#each faceTypes as ft}
        <option value={ft.value}>{ft.label}</option>
      {/each}
    </select>

    {#if faceType === 3}
      <label>Percentage (0-100)</label>
      <input type="number" min="0" max="100" bind:value={percent} />
    {/if}

    {#if faceType === 4}
      <label>Character</label>
      <input type="text" value={character} oninput={handleCharInput} placeholder="A" />
    {/if}

    {#if faceType === 5}
      <div class="raw-controls">
        <label>Panels: {rawSpan}</label>
        <input type="range" min="1" max={Math.min(8 - panelIndex, panelCount - panelIndex)} bind:value={rawSpan} onchange={resizeGrid} />
      </div>
      <div class="raw-grid" style="grid-template-columns: repeat({rawSpan * 8}, 1fr)">
        {#each rawGrid as row, r}
          {#each row as pixel, c}
            <button
              class="raw-pixel"
              class:on={pixel}
              onclick={() => togglePixel(r, c)}
            ></button>
          {/each}
        {/each}
      </div>
    {/if}

    {#if faceType === 1}
      <p class="hint">Clock will use panels {panelIndex + 1}-{Math.min(panelIndex + 4, panelCount)}</p>
    {:else if faceType === 2}
      <p class="hint">Binary will use panels {panelIndex + 1}-{Math.min(panelIndex + 2, panelCount)}</p>
    {/if}

    <div class="buttons">
      <button onclick={apply}>Apply</button>
      <button class="secondary" onclick={onClose}>Cancel</button>
    </div>
  </div>
</div>

<style>
  .overlay {
    position: fixed;
    inset: 0;
    background: rgba(0, 0, 0, 0.6);
    display: flex;
    align-items: center;
    justify-content: center;
    z-index: 200;
  }
  .editor {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    padding: 1.25rem;
    width: 280px;
  }
  .editor.wide {
    width: min(90vw, 420px);
  }
  h3 {
    margin-bottom: 0.75rem;
    font-size: 1rem;
  }
  .hint {
    font-size: 0.8rem;
    color: var(--text-muted);
    margin-top: 0.5rem;
  }
  .buttons {
    display: flex;
    gap: 0.5rem;
    margin-top: 1rem;
  }
  .buttons button {
    flex: 1;
  }
  .secondary {
    background: var(--border);
  }
  .raw-controls {
    margin: 0.5rem 0;
  }
  .raw-grid {
    display: grid;
    gap: 1px;
    background: #111;
    padding: 6px;
    border-radius: 4px;
    margin-top: 0.5rem;
  }
  .raw-pixel {
    width: 100%;
    aspect-ratio: 1;
    min-width: 8px;
    border: none;
    border-radius: 50%;
    background: #222;
    cursor: pointer;
    padding: 0;
  }
  .raw-pixel.on {
    background: #00ff41;
    box-shadow: 0 0 2px #00ff41;
  }
  .raw-pixel:hover {
    background: #333;
  }
  .raw-pixel.on:hover {
    background: #00cc33;
  }
</style>
