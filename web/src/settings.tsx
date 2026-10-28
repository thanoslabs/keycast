import { useEffect, useState, type ChangeEvent } from "react";
import { parseLayout, rows, type Layout } from "./layouts";
import { PlacementEditor, defaultPlacement } from "./placement";
import { VisualKeyEditor } from "./visual-key-editor";

export type Placement = { keyboardX: number; keyboardY: number; mouseX: number; mouseY: number; keyboardScale?: number; mouseScale?: number };
export type Profile = { placement?: Placement; keyWidths?: Record<string, number>; keyHeights?: Record<string, number>; keyOffsets?: Record<string, { x: number; y: number }>; accent: string; opacity: number; scale: number; showMouse: boolean; layout: Layout };
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
  const [selectedKey, setSelectedKey] = useState("Space");
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

  function exportProfile() {
    if (!profile) return;
    const blob = new Blob([JSON.stringify({ version: 1, profile }, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const anchor = document.createElement("a");
    anchor.href = url;
    anchor.download = `keycast-${selected}.json`;
    anchor.click();
    URL.revokeObjectURL(url);
  }

  async function importProfile(event: ChangeEvent<HTMLInputElement>) {
    const file = event.target.files?.[0];
    event.target.value = "";
    if (!file) return;
    if (file.size > 65536) { setStatus("Profile file exceeds 64 KB."); return; }
    try {
      const value: unknown = JSON.parse(await file.text());
      if (!value || typeof value !== "object" || !("version" in value) || value.version !== 1 ||
          !("profile" in value) || !value.profile || typeof value.profile !== "object") throw new Error("Unsupported profile format");
      const candidate = value.profile as Record<string, unknown>;
      if (typeof candidate.accent !== "string" || !/^#[0-9a-fA-F]{6}$/.test(candidate.accent) ||
          typeof candidate.opacity !== "number" || candidate.opacity < 20 || candidate.opacity > 100 ||
          typeof candidate.scale !== "number" || candidate.scale < 50 || candidate.scale > 200 ||
          typeof candidate.showMouse !== "boolean" ||
          (candidate.layout !== "compact" && candidate.layout !== "full")) throw new Error("Invalid profile appearance");
      const name = window.prompt("Name for imported profile (lowercase letters, digits, hyphens):");
      if (!name) return;
      if (!/^[a-z0-9-]{1,32}$/.test(name) || name === "default" || settings.profiles?.[name] ||
          Object.keys(settings.profiles ?? {}).length >= 32) throw new Error("Invalid, duplicate, or excess profile name");
      setSettings(current => ({ ...current, profiles: { ...current.profiles, [name]: candidate as Profile } }));
      setSelected(name);
      setStatus("Imported locally. Save profiles to validate and persist.");
    } catch (error) {
      setStatus(error instanceof Error ? error.message : "Invalid profile JSON");
    }
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
      <div className="profile-actions">
        {profile && <button type="button" onClick={exportProfile}>Export selected profile</button>}
        <label>Import profile JSON <input type="file" accept=".json,application/json" onChange={event => void importProfile(event)} /></label>
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
      {selected !== "default" && layout === "full" && <div className="key-customization">
        <h2>Visual keyboard editor</h2>
        <VisualKeyEditor selected={selectedKey} onSelect={setSelectedKey}
          widths={profile?.keyWidths} heights={profile?.keyHeights} offsets={profile?.keyOffsets}
          onOffsetsChange={next => edit({ keyOffsets: next })}
          onWidthsChange={next => edit({ keyWidths: next })}
          onHeightsChange={next => edit({ keyHeights: next })} />
        <label>Key
          <select value={selectedKey} onChange={event => setSelectedKey(event.target.value)}>
            {[...new Set(rows.flat())].map(key => <option key={key} value={key}>{key}</option>)}
          </select>
        </label>
        <label>Width: {profile?.keyWidths?.[selectedKey] ?? 100}%
          <input type="range" min={50} max={300} step={5}
            value={profile?.keyWidths?.[selectedKey] ?? 100}
            onChange={event => edit({ keyWidths: { ...profile?.keyWidths, [selectedKey]: Number(event.target.value) } })} />
        </label>
        <label>Height: {profile?.keyHeights?.[selectedKey] ?? 100}%
          <input type="range" min={50} max={300} step={5}
            value={profile?.keyHeights?.[selectedKey] ?? 100}
            onChange={event => edit({ keyHeights: { ...profile?.keyHeights, [selectedKey]: Number(event.target.value) } })} />
        </label>
        <fieldset className="key-offset-controls">
          <legend>Key position offset (px)</legend>
          {(["x", "y"] as const).map(axis => <label key={axis}>{axis.toUpperCase()}
            <input type="number" min={-300} max={300} step={1}
              value={profile?.keyOffsets?.[selectedKey]?.[axis] ?? 0}
              onChange={event => {
                const n = Number(event.target.value);
                if (!Number.isFinite(n)) return;
                edit({ keyOffsets: {
                  ...profile?.keyOffsets,
                  [selectedKey]: { x: profile?.keyOffsets?.[selectedKey]?.x ?? 0,
                    y: profile?.keyOffsets?.[selectedKey]?.y ?? 0, [axis]: Math.max(-300, Math.min(300, Math.round(n))) }
                } });
              }} />
          </label>)}
          <button type="button" onClick={() => {
            const offsets = { ...profile?.keyOffsets };
            delete offsets[selectedKey];
            edit({ keyOffsets: offsets });
          }}>Reset position</button>
        </fieldset>
        <button type="button" onClick={() => {
          const offsets = { ...profile?.keyOffsets };
          const widths = { ...profile?.keyWidths };
          const heights = { ...profile?.keyHeights };
          delete offsets[selectedKey];
          delete widths[selectedKey];
          delete heights[selectedKey];
          edit({ keyOffsets: offsets, keyWidths: widths, keyHeights: heights });
        }}>Reset all selected key customizations</button>
        <button type="button" onClick={() => {
          const heights = { ...profile?.keyHeights };
          delete heights[selectedKey];
          edit({ keyHeights: heights });
        }}>Reset selected key height</button>
        <button type="button" onClick={() => {
          const widths = { ...profile?.keyWidths };
          delete widths[selectedKey];
          edit({ keyWidths: widths });
        }}>Reset selected key</button>
      </div>}
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
