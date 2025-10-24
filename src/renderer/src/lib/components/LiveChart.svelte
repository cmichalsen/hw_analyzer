<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import Chart from 'chart.js/auto';
	import { backend, type WebSocketMessage } from '$lib/backend';

	export let connected = false;

	let canvas: HTMLCanvasElement;
	let chart: Chart | null = null;
	let unsubscribe: (() => void) | null = null;
	let maxDataPoints = 100;
	let paused = false;

	onMount(() => {
		initChart();
		unsubscribe = backend.onMessage(handleMessage);
	});

	function initChart() {
		if (!canvas) return;

		chart = new Chart(canvas, {
			type: 'line',
			data: {
				labels: [],
				datasets: [
					{
						label: 'Serial Data',
						data: [],
						borderColor: '#3b82f6',
						backgroundColor: 'rgba(59, 130, 246, 0.1)',
						borderWidth: 2,
						pointRadius: 0,
						tension: 0.4,
						fill: true
					}
				]
			},
			options: {
				responsive: true,
				maintainAspectRatio: false,
				animation: false,
				scales: {
					x: {
						display: true,
						title: {
							display: true,
							text: 'Time'
						},
						ticks: {
							maxTicksLimit: 10
						}
					},
					y: {
						display: true,
						title: {
							display: true,
							text: 'Value'
						},
						beginAtZero: false
					}
				},
				plugins: {
					legend: {
						display: true,
						position: 'top'
					},
					tooltip: {
						enabled: true,
						mode: 'index',
						intersect: false
					}
				},
				interaction: {
					mode: 'nearest',
					axis: 'x',
					intersect: false
				}
			}
		});
	}

	function handleMessage(msg: WebSocketMessage) {
		if (msg.type === 'rx' && !paused) {
			const data = msg.data as string;
			// Try to parse numeric values from the received data
			const lines = data.trim().split(/\r?\n/);

			for (const line of lines) {
				const trimmed = line.trim();
				// Try to extract numbers from the line
				const numbers = trimmed.match(/-?\d+\.?\d*/g);

				if (numbers) {
					for (const numStr of numbers) {
						const num = parseFloat(numStr);
						if (!isNaN(num)) {
							addDataPoint(num);
						}
					}
				}
			}
		}
	}

	function addDataPoint(value: number) {
		if (!chart) return;

		const now = new Date();
		const timeLabel = now.toLocaleTimeString('en-US', {
			hour12: false,
			hour: '2-digit',
			minute: '2-digit',
			second: '2-digit'
		});

		chart.data.labels?.push(timeLabel);
		chart.data.datasets[0].data.push(value);

		// Keep only the last N points
		if (chart.data.labels && chart.data.labels.length > maxDataPoints) {
			chart.data.labels.shift();
			chart.data.datasets[0].data.shift();
		}

		chart.update();
	}

	function clearChart() {
		if (!chart) return;
		chart.data.labels = [];
		chart.data.datasets[0].data = [];
		chart.update();
	}

	function togglePause() {
		paused = !paused;
	}

	onDestroy(() => {
		if (chart) {
			chart.destroy();
		}
		if (unsubscribe) {
			unsubscribe();
		}
	});
</script>

<div class="chart-container">
	<div class="chart-header">
		<h3>Live Data Graph</h3>
		<div class="controls">
			<label class="control-item">
				Max Points:
				<input
					type="number"
					bind:value={maxDataPoints}
					min="10"
					max="1000"
					step="10"
					class="w-20 rounded border px-2 py-1 text-sm"
				/>
			</label>
			<button on:click={togglePause} class="btn-small" class:paused>
				{paused ? '▶️ Resume' : '⏸️ Pause'}
			</button>
			<button on:click={clearChart} class="btn-small">🗑️ Clear</button>
		</div>
	</div>
	<div class="chart-wrapper">
		{#if connected}
			<canvas bind:this={canvas}></canvas>
		{:else}
			<div class="chart-placeholder">
				<p>Connect to a serial port to view live data</p>
			</div>
		{/if}
	</div>
</div>

<style>
	.chart-container {
		display: flex;
		flex-direction: column;
		height: 100%;
		background: white;
		border-radius: 0.5rem;
		box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
		overflow: hidden;
	}

	:global(.dark) .chart-container {
		background: #1f2937;
	}

	.chart-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: 1rem;
		border-bottom: 1px solid #e5e7eb;
		background: #f9fafb;
	}

	:global(.dark) .chart-header {
		border-bottom-color: #374151;
		background: #111827;
	}

	h3 {
		margin: 0;
		font-size: 1.125rem;
		font-weight: 600;
	}

	.controls {
		display: flex;
		gap: 0.75rem;
		align-items: center;
	}

	.control-item {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		font-size: 0.875rem;
	}

	.chart-wrapper {
		flex: 1;
		padding: 1rem;
		position: relative;
		min-height: 0;
	}

	.chart-placeholder {
		display: flex;
		align-items: center;
		justify-content: center;
		height: 100%;
		color: #9ca3af;
		font-size: 1.125rem;
	}

	.btn-small {
		padding: 0.375rem 0.75rem;
		font-size: 0.875rem;
		border-radius: 0.375rem;
		background: #6b7280;
		color: white;
		border: none;
		cursor: pointer;
		transition: background-color 0.2s;
	}

	.btn-small:hover {
		background: #4b5563;
	}

	.btn-small.paused {
		background: #f59e0b;
	}

	.btn-small.paused:hover {
		background: #d97706;
	}
</style>
