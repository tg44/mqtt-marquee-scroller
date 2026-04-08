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

export interface PanelFace {
  type: number;
  percent: number;
  char: string;
}

export interface Dashboard {
  device: {
    hostname: string;
    version: string;
    uptimeSeconds: number;
    resetReason: string;
    freeHeap: number;
    cpuMhz: number;
  };
  wifi: {
    ssid: string;
    ip: string;
    signal: number;
  };
  time: {
    formatted: string;
    hour: number;
    minute: number;
    second: number;
    is24Hour: boolean;
  };
  display: {
    on: boolean;
    panels: number;
    brightness: number;
  };
  mqtt: {
    enabled: boolean;
    connected: boolean;
    url: string;
    port: number;
    deviceId: string;
    topic: string;
    faceTopic: string;
    lastMessage: string;
    scrollBuffer: string;
    panelFaces: PanelFace[];
  };
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
  getDashboard: () => request<Dashboard>('/api/dashboard'),
  getSettings: () => request<Settings>('/api/settings'),
  saveSettings: (data: Settings) => post<{ ok: boolean }>('/api/settings', data),
  getMqtt: () => request<MqttConfig>('/api/mqtt'),
  saveMqtt: (data: MqttConfig) => post<{ ok: boolean; restarting: boolean }>('/api/mqtt', data),
  getStatus: () => request<Status>('/api/status'),
  toggleDisplay: () => post<{ displayOn: boolean }>('/api/display/toggle'),
  getDisplayPixels: () => request<{ panels: number; pixels: number[][] }>('/api/display/pixels'),
  setBrightness: (brightness: number) => post<{ ok: boolean }>('/api/brightness', { brightness }),
  setFaces: (panels: { t: number; p: number; c: string }[]) => post<{ ok: boolean }>('/api/faces', { panels }),
  postRaw: (panel: number, pixels: number[][]) => post<{ ok: boolean }>('/api/raw', { panel, pixels }),
  refreshNtp: () => post<{ ok: boolean }>('/api/ntp/refresh'),
  reset: () => post<{ ok: boolean; restarting: boolean }>('/api/reset'),
  forgetWifi: () => post<{ ok: boolean; restarting: boolean }>('/api/forgetwifi'),
};
