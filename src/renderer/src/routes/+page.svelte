<script lang="ts">
	import { onMount } from 'svelte';
	import PortSelector from '$lib/components/PortSelector.svelte';
	import SerialConsole from '$lib/components/SerialConsole.svelte';
	import LiveChart from '$lib/components/LiveChart.svelte';
	import { backend } from '$lib/backend';

	let connected = false;
	let activeTab: 'console' | 'graph' = 'console';

	function handlePortSelected(port: string, baud: number) {
		backend.openPort(port, baud);
		connected = true;
	}

	function handleDisconnect() {
		connected = false;
	}

	onMount(() => {
		console.log('HW Analyzer loaded');
	});
</script>

<div class="app-container">
	<header>
		<h1>HW Analyzer</h1>
		<p class="subtitle">Serial / I²C / SPI Debugging Tool</p>
	</header>

	<main>
		<PortSelector {connected} onPortSelected={handlePortSelected} />

		<div class="tabs">
			<button
				class="tab"
				class:active={activeTab === 'console'}
				on:click={() => (activeTab = 'console')}
			>
				📟 Console
			</button>
			<button
				class="tab"
				class:active={activeTab === 'graph'}
				on:click={() => (activeTab = 'graph')}
			>
				📊 Live Graph
			</button>
		</div>

		<div class="tab-content">
			{#if activeTab === 'console'}
				<SerialConsole {connected} onDisconnect={handleDisconnect} />
			{:else}
				<LiveChart {connected} />
			{/if}
		</div>
	</main>
</div>

<style>
	.app-container {
		display: flex;
		flex-direction: column;
		height: 100vh;
		background: #f3f4f6;
	}

	:global(.dark) .app-container {
		background: #111827;
	}

	header {
		padding: 1.5rem 2rem;
		background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
		color: white;
		box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
	}

	h1 {
		margin: 0;
		font-size: 1.875rem;
		font-weight: 700;
	}

	.subtitle {
		margin: 0.25rem 0 0 0;
		font-size: 0.875rem;
		opacity: 0.9;
	}

	main {
		flex: 1;
		display: flex;
		flex-direction: column;
		overflow: hidden;
	}

	.tabs {
		display: flex;
		gap: 0.25rem;
		padding: 0 1rem;
		background: #e5e7eb;
		border-bottom: 2px solid #d1d5db;
	}

	:global(.dark) .tabs {
		background: #1f2937;
		border-bottom-color: #374151;
	}

	.tab {
		padding: 0.75rem 1.5rem;
		background: transparent;
		border: none;
		border-bottom: 2px solid transparent;
		color: #6b7280;
		cursor: pointer;
		font-size: 0.95rem;
		font-weight: 500;
		transition: all 0.2s;
		margin-bottom: -2px;
	}

	.tab:hover {
		color: #374151;
		background: rgba(0, 0, 0, 0.05);
	}

	:global(.dark) .tab:hover {
		color: #d1d5db;
		background: rgba(255, 255, 255, 0.05);
	}

	.tab.active {
		color: #667eea;
		border-bottom-color: #667eea;
		background: white;
	}

	:global(.dark) .tab.active {
		background: #111827;
	}

	.tab-content {
		flex: 1;
		overflow: hidden;
		padding: 1rem;
	}
</style>
