import { useEffect, useState } from "react";
import { parseLayout, type Layout } from "./layouts";
import { PlacementEditor, defaultPlacement } from "./placement";

export type Placement = { keyboardX: number; keyboardY: number; mouseX: number; mouseY: number; keyboardScale?: number; mouseScale?: number };
export type Profile = { placement?: Placement; accent: string; opacity: number; scale: number; showMouse: boolean; layout: Layout };
export type Settings = { version: number; accent: string; opacity: number; scale: number; showMouse: boolean; profiles?: Record<string, Profile> };
export const defaultSettings: Settings = { version: 1, accent: "#6d53c5", opacity: 85, scale: 100, showMouse: true };
export async function loadSettings(): Promise<Settings> {
  const response = await fetch("/api/v1/settings", { cache: "no-store" });
  if (!response.ok) throw new Error("Settings server unavailable");
  return await response.json() as Settings;
}

export function SettingsDashboard() {
  const [settings, setSettings] = useState<Settings>(defaultSettings);
  const [status, setStatus] = useState("Loading settings…");
  const [selected, setSelected] = useState("default");
  const [newName, setNewName] = useState("");
  const [defaultLayout, setDefaultLayout] = useState<Layout>("compact");
  const profile = selected === "default" ? undefined : settings.profiles?.[selected];
  const appearance = profile ?? { ...settings, layout: defaultLayout };
  const layout = profile?.layout ?? defaultLayout;
  const placement = profile?.placement ?? defaultPlacement;
  const overlayUrl = `${window.location.origin}/overlay/default?profile=${encodeURIComponent(selected)}${selected === "default" ? `&layout=${layout}` : ""}`;

  useEffect(() => {
    void loadSettings().then(value => { setSettings(value); setStatus(""); })
      .catch(() => setStatus("Start the Go server to edit settings."));
  }, []);

  function edit(change: Partial<Profile>) {
    if (selected === "default") {
      if (change.layout) setDefaultLayout(change.layout);
      setSettings(current => ({ ...current, ...change, profiles: current.profiles }));
    } else {
      setSettings(current => ({
        ...current,
        profiles: { ...current.profiles, [selected]: { ...current.profiles![selected], ...change } }
      }));
    }
    setStatus("Unsaved changes");
  }

  function addProfile() {
    const name = newName.trim();
    if (!/^[a-z0-9-]{1,32}$/.test(name) || name === "default" || settings.profiles?.[name]) {
      setStatus("Use a unique lowercase name (letters, numbers, hyphens; maximum 32).");
      return;
    }
    if (Object.keys(settings.profiles ?? {}).length >= 32) {
      setStatus("Maximum of 32 named profiles.");
      return;
    }
    setSettings(current => ({
      ...current,
      profiles: { ...current.profiles, [name]: { accent: current.accent, opacity: current.opacity, scale: current.scale, showMouse: current.showMouse, layout: defaultLayout } }
    }));
    setSelected(name);
    setNewName("");
    setStatus("Unsaved changes");
  }

  function removeProfile() {
    if (selected === "default") return;
    const next = { ...settings.profiles };
    delete next[selected];
    setSettings(current => ({ ...current, profiles: next }));
    setSelected("default");
    setStatus("Unsaved changes");
  }

  async function save() {
    setStatus("Saving…");
    try {
      const response = await fetch("/api/v1/settings", {
        method: "PUT", headers: { "Content-Type": "application/json" }, body: JSON.stringify(settings)
      });
      if (!response.ok) throw new Error(await response.text());
      setSettings(await response.json() as Settings);
      setStatus("Saved");
    } catch (error) {
      setStatus(error instanceof Error ? error.message : "Save failed");
    }
  }

  return <main className="dashboard">
    <h1>Keycast</h1><p>Local overlay profiles</p>
    <div className="settings-form">
      <label>Profile
        <select value={selected} onChange={event => setSelected(event.target.value)}>
          <option value="default">Default</option>
          {Object.keys(settings.profiles ?? {}).sort().map(name => <option key={name} value={name}>{name}</option>)}
        </select>
      </label>
      <div className="profile-actions">
        <input aria-label="New profile name" placeholder="new-profile" value={newName} onChange={event => setNewName(event.target.value)} />
        <button type="button" onClick={addProfile}>Add profile</button>
        {selected !== "default" && <button type="button" onClick={removeProfile}>Delete</button>}
      </div>
      <label>Keyboard layout
        <select value={layout} onChange={event => edit({ layout: parseLayout(event.target.value) })}>
          <option value="compact">Compact</option><option value="full">Full keyboard</option>
        </select>
      </label>
      <label>Accent color <input type="color" value={appearance.accent} onChange={event => edit({ accent: event.target.value })} /></label>
      <label>Opacity: {appearance.opacity}% <input type="range" min="20" max="100" value={appearance.opacity} onChange={event => edit({ opacity: Number(event.target.value) })} /></label>
      <label>Scale: {appearance.scale}% <input type="range" min="50" max="200" value={appearance.scale} onChange={event => edit({ scale: Number(event.target.value) })} /></label>
      <label><input type="checkbox" checked={appearance.showMouse} onChange={event => edit({ showMouse: event.target.checked })} /> Show mouse</label>
      {selected !== "default" && <PlacementEditor value={placement} onChange={next => edit({ placement: next })} />}
      <button type="button" onClick={() => void save()}>Save profiles</button>
      <span role="status">{status}</span>
    </div>
    <section className="layout-preview">
      <h2>OBS preview</h2>
      <iframe title="OBS overlay preview" src={overlayUrl} className="overlay-preview-frame" />
      <p><a href={overlayUrl} target="_blank" rel="noreferrer">Open selected OBS overlay</a></p>
      <p className="hint">Browser source: <code>{overlayUrl}</code></p>
      <button type="button" onClick={() => void navigator.clipboard.writeText(overlayUrl).then(() => setStatus("URL copied")).catch(() => setStatus("Unable to copy URL"))}>Copy OBS URL</button>
    </section>
  </main>;
}
