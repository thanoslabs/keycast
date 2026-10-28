import { useEffect, useState } from "react";
import { parseLayout, type Layout } from "./layouts";

export type Settings = { version: number; accent: string; opacity: number; scale: number; showMouse: boolean };
export const defaultSettings: Settings = { version: 1, accent: "#6d53c5", opacity: 85, scale: 100, showMouse: true };
export async function loadSettings(): Promise<Settings> {
  const response = await fetch("/api/v1/settings", { cache: "no-store" });
  if (!response.ok) throw new Error("Settings server unavailable");
  return await response.json() as Settings;
}
export function SettingsDashboard() {
  const [settings, setSettings] = useState<Settings>(defaultSettings);
  const [status, setStatus] = useState("Loading settings…");
  const [layout, setLayout] = useState<Layout>(() => parseLayout(new URLSearchParams(window.location.search).get("layout")));
  const overlayUrl = `${window.location.origin}/overlay/default?layout=${layout}`;
  useEffect(() => { void loadSettings().then(value => { setSettings(value); setStatus(""); }).catch(() => setStatus("Start the Go server to edit settings.")); }, []);
  async function save() {
    setStatus("Saving…");
    try {
      const response = await fetch("/api/v1/settings", { method: "PUT", headers: { "Content-Type": "application/json" }, body: JSON.stringify(settings) });
      if (!response.ok) throw new Error(await response.text());
      setSettings(await response.json() as Settings);
      setStatus("Saved");
    } catch (error) { setStatus(error instanceof Error ? error.message : "Save failed"); }
  }
  return <main className="dashboard"><h1>Keycast</h1><p>Local overlay configuration</p>
    <div className="settings-form">
      <label>Accent color <input type="color" value={settings.accent} onChange={event => setSettings({ ...settings, accent: event.target.value })} /></label>
      <label>Opacity: {settings.opacity}% <input type="range" min="20" max="100" value={settings.opacity} onChange={event => setSettings({ ...settings, opacity: Number(event.target.value) })} /></label>
      <label>Scale: {settings.scale}% <input type="range" min="50" max="200" value={settings.scale} onChange={event => setSettings({ ...settings, scale: Number(event.target.value) })} /></label>
      <label><input type="checkbox" checked={settings.showMouse} onChange={event => setSettings({ ...settings, showMouse: event.target.checked })} /> Show mouse</label>
      <button type="button" onClick={() => void save()}>Save settings</button>
      <span role="status">{status}</span>
    </div>
    <section className="layout-preview"><h2>Keyboard layout preview</h2>
      <label htmlFor="keyboard-layout">Layout</label>
      <select id="keyboard-layout" value={layout} onChange={event => setLayout(parseLayout(event.target.value))}>
        <option value="compact">Compact pressed keys</option>
        <option value="full">Full keyboard</option>
      </select>
      <iframe title="OBS overlay preview" src={`/overlay/default?layout=${layout}`} className="overlay-preview-frame" />
      <p><a href={overlayUrl} target="_blank" rel="noreferrer">Open selected OBS overlay</a></p>
      <p className="hint">Browser source: <code>{overlayUrl}</code></p>
    </section>
  </main>;
}
