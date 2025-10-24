<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { backend, type SerialPortInfo } from '$lib/backend';

	export let onPortSelected: (port: string, baud: number) => void = () => {};
	export let connected = false;

	let ports: SerialPortInfo[] = [];
	let selectedPort = '';
	let baudRate = 115200;
	let loading = false;
	let error = '';

	const baudRates = [9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600];

	onMount(async () => {
		try {
			await backend.connect();
			await refreshPorts();
		} catch (err) {
			error = 'Failed to connect to backend. Make sure the backend is running.';
			console.error(err);
		}
	});

	async function refreshPorts() {
		loading = true;
		error = '';
		try {
			ports = await backend.listPorts();
			if (ports.length > 0 && !selectedPort) {
				selectedPort = ports[0].name;
			}
		} catch (err) {
			error = 'Failed to list ports';
			console.error(err);
		} finally {
			loading = false;
		}
	}

	function handleConnect() {
		if (!selectedPort) {
			error = 'Please select a port';
			return;
		}
		onPortSelected(selectedPort, baudRate);
	}

	onDestroy(() => {
		backend.disconnect();
	});
</script>

<div class="port-selector">
	<div class="flex items-center gap-2">
		<div class="flex-1">
			<label for="port" class="mb-1 block text-sm font-medium">Serial Port</label>
			<select
				id="port"
				bind:value={selectedPort}
				disabled={connected || loading}
				class="w-full rounded-md border bg-white px-3 py-2 disabled:opacity-50 dark:bg-gray-800"
			>
				<option value="" disabled>Select a port...</option>
				{#each ports as port (port.name)}
					<option value={port.name}>{port.name} - {port.desc}</option>
				{/each}
			</select>
		</div>

		<div class="w-32">
			<label for="baud" class="mb-1 block text-sm font-medium">Baud Rate</label>
			<select
				id="baud"
				bind:value={baudRate}
				disabled={connected}
				class="w-full rounded-md border bg-white px-3 py-2 disabled:opacity-50 dark:bg-gray-800"
			>
				{#each baudRates as rate (rate)}
					<option value={rate}>{rate}</option>
				{/each}
			</select>
		</div>

		<div class="flex items-end gap-2">
			<button
				on:click={refreshPorts}
				disabled={connected || loading}
				class="rounded-md bg-gray-500 px-4 py-2 text-white hover:bg-gray-600 disabled:cursor-not-allowed disabled:opacity-50"
			>
				{loading ? '⟳' : '🔄'} Refresh
			</button>

			<button
				on:click={handleConnect}
				disabled={!selectedPort || connected}
				class="rounded-md bg-green-600 px-4 py-2 text-white hover:bg-green-700 disabled:cursor-not-allowed disabled:opacity-50"
			>
				Connect
			</button>
		</div>
	</div>

	{#if error}
		<div class="mt-2 rounded bg-red-100 p-2 text-sm text-red-700 dark:bg-red-900 dark:text-red-200">
			{error}
		</div>
	{/if}
</div>

<style>
	.port-selector {
		padding: 1rem;
		border-bottom: 1px solid #e5e7eb;
		background-color: #f9fafb;
	}

	:global(.dark) .port-selector {
		border-bottom-color: #374151;
		background-color: #1f2937;
	}
</style>
