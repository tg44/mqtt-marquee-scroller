<script lang="ts">
  import Nav from './components/Nav.svelte';
  import Dashboard from './components/Dashboard.svelte';
  import GeneralSettings from './components/GeneralSettings.svelte';
  import MqttSettings from './components/MqttSettings.svelte';
  import Actions from './components/Actions.svelte';
  import Footer from './components/Footer.svelte';

  let currentView = $state('dashboard');
</script>

<main>
  <h1>Marquee Scroller</h1>
  <Nav current={currentView} onNavigate={(v) => (currentView = v)} />

  {#if currentView === 'dashboard'}
    <Dashboard />
  {:else if currentView === 'settings'}
    <GeneralSettings />
  {:else if currentView === 'mqtt'}
    <MqttSettings />
  {:else if currentView === 'actions'}
    <Actions />
  {/if}

  <Footer />
</main>
