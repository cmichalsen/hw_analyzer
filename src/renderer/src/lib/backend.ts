// WebSocket client for communicating with C++ backend

export interface SerialPortInfo {
	name: string;
	desc: string;
}

export interface WebSocketMessage {
	type: 'ports' | 'rx' | 'status' | 'error';
	data?: SerialPortInfo[] | string;
	message?: string;
}

type MessageCallback = (message: WebSocketMessage) => void;

class BackendClient {
	private ws: WebSocket | null = null;
	private callbacks: Set<MessageCallback> = new Set();
	private reconnectTimer: number | null = null;
	private url: string;

	constructor(url = 'ws://localhost:9001') {
		this.url = url;
	}

	connect(): Promise<void> {
		return new Promise((resolve, reject) => {
			try {
				this.ws = new WebSocket(this.url);

				this.ws.onopen = () => {
					console.log('[Backend] Connected to backend');
					resolve();
				};

				this.ws.onmessage = (event) => {
					try {
						const message: WebSocketMessage = JSON.parse(event.data);
						this.callbacks.forEach((cb) => cb(message));
					} catch (err) {
						console.error('[Backend] Failed to parse message:', err);
					}
				};

				this.ws.onerror = (error) => {
					console.error('[Backend] WebSocket error:', error);
					reject(error);
				};

				this.ws.onclose = (event) => {
					console.log('[Backend] Disconnected from backend');
					this.attemptReconnect();
				};
			} catch (err) {
				reject(err);
			}
		});
	}

	private attemptReconnect() {
		if (this.reconnectTimer) return;

		this.reconnectTimer = window.setTimeout(() => {
			console.log('Attempting to reconnect...');
			this.reconnectTimer = null;
			this.connect().catch(() => {
				// Will try again on next timer
			});
		}, 3000);
	}

	disconnect() {
		if (this.reconnectTimer) {
			clearTimeout(this.reconnectTimer);
			this.reconnectTimer = null;
		}
		if (this.ws) {
			this.ws.close();
			this.ws = null;
		}
	}

	onMessage(callback: MessageCallback) {
		this.callbacks.add(callback);
		return () => this.callbacks.delete(callback);
	}

	private send(data: object) {
		if (this.ws && this.ws.readyState === WebSocket.OPEN) {
			this.ws.send(JSON.stringify(data));
		} else {
			console.error('WebSocket not connected');
		}
	}

	// API Methods
	async listPorts(): Promise<SerialPortInfo[]> {
		return new Promise((resolve) => {
			const unsubscribe = this.onMessage((msg) => {
				if (msg.type === 'ports' && Array.isArray(msg.data)) {
					unsubscribe();
					resolve(msg.data as SerialPortInfo[]);
				}
			});
			this.send({ cmd: 'list' });

			// Timeout after 5 seconds
			setTimeout(() => {
				unsubscribe();
				resolve([]);
			}, 5000);
		});
	}

	openPort(port: string, baud: number) {
		this.send({ cmd: 'open', port, baud });
	}

	closePort() {
		this.send({ cmd: 'close' });
	}

	writeData(data: string) {
		this.send({ cmd: 'write', data });
	}
}

// Export singleton instance
export const backend = new BackendClient();
