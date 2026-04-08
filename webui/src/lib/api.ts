export interface Settings {
  utcOffsetHours: number;
  is24Hour: boolean;
  isPM: boolean;
  flashSeconds: boolean;
  startTime: string;
  endTime: string;
  brightness: number;
  scrollSpeed: number;
  refreshMinutes: number;
  basicAuth: boolean;
  username: string;
  password: string;
}

export interface MqttConfig {
  enabled: boolean;
  deviceId: string;
  url: string;
  port: number;
  topic: string;
  faceTopic: string;
}

export interface Status {
  version: string;
  wifiSignal: number;
  displayOn: boolean;
}

async function request<T>(url: string, options?: RequestInit): Promise<T> {
  const res = await fetch(url, { credentials: 'same-origin', ...options });
  if (!res.ok) throw new Error(`${res.status} ${res.statusText}`);
  return res.json();
}

function post<T>(url: string, body?: unknown): Promise<T> {
  return request<T>(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: body ? JSON.stringify(body) : undefined,
  });
}

export const api = {
  getSettings: () => request<Settings>('/api/settings'),
  saveSettings: (data: Settings) => post<{ ok: boolean }>('/api/settings', data),
  getMqtt: () => request<MqttConfig>('/api/mqtt'),
  saveMqtt: (data: MqttConfig) => post<{ ok: boolean; restarting: boolean }>('/api/mqtt', data),
  getStatus: () => request<Status>('/api/status'),
  toggleDisplay: () => post<{ displayOn: boolean }>('/api/display/toggle'),
  refreshNtp: () => post<{ ok: boolean }>('/api/ntp/refresh'),
  reset: () => post<{ ok: boolean; restarting: boolean }>('/api/reset'),
  forgetWifi: () => post<{ ok: boolean; restarting: boolean }>('/api/forgetwifi'),
};
