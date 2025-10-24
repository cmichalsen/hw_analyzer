<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { backend, type WebSocketMessage } from '$lib/backend';

	export let connected = false;
	export let onDisconnect: () => void = () => {};

	let messages: Array<{ timestamp: Date; type: string; text: string }> = [];
	let inputText = '';
	let autoScroll = true;
	let consoleElement: HTMLDivElement;
	let unsubscribe: (() => void) | null = null;

	onMount(() => {
		unsubscribe = backend.onMessage(handleMessage);
	});

	function handleMessage(msg: WebSocketMessage) {
		const timestamp = new Date();

		switch (msg.type) {
			case 'rx':
				addMessage(timestamp, 'rx', msg.data as string);
				break;
			case 'status':
				addMessage(timestamp, 'status', msg.message || '');
				break;
			case 'error':
				addMessage(timestamp, 'error', msg.message || '');
				break;
		}
	}

	function addMessage(timestamp: Date, type: string, text: string) {
		messages = [...messages, { timestamp, type, text }];
		if (autoScroll && consoleElement) {
			setTimeout(() => {
				consoleElement.scrollTop = consoleElement.scrollHeight;
			}, 0);
		}
	}

	function send() {
		if (!inputText.trim() || !connected) return;

		const timestamp = new Date();
		addMessage(timestamp, 'tx', inputText);
		backend.writeData(inputText + '\n');
		inputText = '';
	}

	function handleKeyPress(event: KeyboardEvent) {
		if (event.key === 'Enter' && !event.shiftKey) {
			event.preventDefault();
			send();
		}
	}

	function disconnect() {
		backend.closePort();
		onDisconnect();
	}

	function clearConsole() {
		messages = [];
	}

	function formatTimestamp(date: Date): string {
		return date.toLocaleTimeString('en-US', {
			hour12: false,
			hour: '2-digit',
			minute: '2-digit',
			second: '2-digit',
			fractionalSecondDigits: 3
		});
	}

	onDestroy(() => {
		if (unsubscribe) unsubscribe();
	});
</script>

<div class="serial-console">
	<div class="console-header">
		<div class="flex items-center gap-2">
			<span class="status-indicator {connected ? 'connected' : 'disconnected'}"></span>
			<span class="text-sm font-medium">{connected ? 'Connected' : 'Disconnected'}</span>
		</div>
		<div class="flex gap-2">
			<label class="flex items-center gap-2 text-sm">
				<input type="checkbox" bind:checked={autoScroll} />
				Auto-scroll
			</label>
			<button on:click={clearConsole} class="btn-small">Clear</button>
			{#if connected}
				<button on:click={disconnect} class="btn-small btn-danger">Disconnect</button>
			{/if}
		</div>
	</div>

	<div bind:this={consoleElement} class="console-output">
		{#each messages as msg, i (i)}
			<div class="message message-{msg.type}">
				<span class="timestamp">{formatTimestamp(msg.timestamp)}</span>
				<span class="type-badge">{msg.type.toUpperCase()}</span>
				<span class="text">{msg.text}</span>
			</div>
		{/each}
	</div>

	<div class="console-input">
		<input
			type="text"
			bind:value={inputText}
			on:keypress={handleKeyPress}
			disabled={!connected}
			placeholder={connected ? 'Type message and press Enter...' : 'Connect to a port first...'}
			class="flex-1 rounded-md border px-3 py-2 disabled:opacity-50 dark:bg-gray-800"
		/>
		<button
			on:click={send}
			disabled={!connected || !inputText.trim()}
			class="rounded-md bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 disabled:cursor-not-allowed disabled:opacity-50"
		>
			Send
		</button>
	</div>
</div>

<style>
	.serial-console {
		display: flex;
		flex-direction: column;
		height: 100%;
		background: white;
	}

	:global(.dark) .serial-console {
		background: #111827;
	}

	.console-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: 0.75rem 1rem;
		border-bottom: 1px solid #e5e7eb;
		background: #f9fafb;
	}

	:global(.dark) .console-header {
		border-bottom-color: #374151;
		background: #1f2937;
	}

	.status-indicator {
		width: 8px;
		height: 8px;
		border-radius: 50%;
		background: #9ca3af;
	}

	.status-indicator.connected {
		background: #10b981;
		box-shadow: 0 0 8px #10b981;
	}

	.console-output {
		flex: 1;
		overflow-y: auto;
		padding: 1rem;
		font-family: 'Courier New', monospace;
		font-size: 0.875rem;
		background: #000;
		color: #0f0;
	}

	.message {
		margin-bottom: 0.25rem;
		display: flex;
		gap: 0.5rem;
	}

	.timestamp {
		color: #6b7280;
		flex-shrink: 0;
	}

	.type-badge {
		padding: 0 0.375rem;
		border-radius: 0.25rem;
		font-size: 0.75rem;
		font-weight: 600;
		flex-shrink: 0;
	}

	.message-rx .type-badge {
		background: #10b981;
		color: white;
	}

	.message-tx .type-badge {
		background: #3b82f6;
		color: white;
	}

	.message-status .type-badge {
		background: #f59e0b;
		color: white;
	}

	.message-error .type-badge {
		background: #ef4444;
		color: white;
	}

	.message-rx .text {
		color: #10b981;
	}

	.message-tx .text {
		color: #3b82f6;
	}

	.message-status .text {
		color: #f59e0b;
	}

	.message-error .text {
		color: #ef4444;
	}

	.console-input {
		display: flex;
		gap: 0.5rem;
		padding: 1rem;
		border-top: 1px solid #e5e7eb;
	}

	:global(.dark) .console-input {
		border-top-color: #374151;
	}

	.btn-small {
		padding: 0.25rem 0.75rem;
		font-size: 0.875rem;
		border-radius: 0.25rem;
		background: #6b7280;
		color: white;
		border: none;
		cursor: pointer;
	}

	.btn-small:hover {
		background: #4b5563;
	}

	.btn-danger {
		background: #ef4444;
	}

	.btn-danger:hover {
		background: #dc2626;
	}
</style>
