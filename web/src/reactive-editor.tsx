import { useEffect, useState } from "react";
import type { ReactiveFrame, ReactiveLayer, ReactiveScene } from "./settings";
import { triggerMatches } from "./reactive-triggers";

type Asset = { id: string; url: string; name?: string };
type Props = { value?: ReactiveScene; onChange: (scene: ReactiveScene) => void };
const clamp = (value: number, min: number, max: number) => Math.max(min, Math.min(max, Math.round(value) || min));

export function ReactiveSceneEditor({ value, onChange }: Props) {
  const [assets, setAssets] = useState<Asset[]>([]);
  const [selected, setSelected] = useState("");
  const [held, setHeld] = useState<string[]>([]);
  const [preview, setPreview] = useState(false);
  const [uploadStatus, setUploadStatus] = useState("");
  const [advanced, setAdvanced] = useState(false);
  const [animationPreview, setAnimationPreview] = useState<{ layerId: string; phase: "pressFrames" | "releaseFrames"; index: number } | null>(null);
  async function uploadPart(file: File, field: "idle" | "active" | "pressFrames" | "releaseFrames", previousFrames?: ReactiveFrame[]) {
    if (!current) return;
    if (file.size > 4 * 1024 * 1024) { setUploadStatus("Image must be under 4 MB"); return; }
    try {
      const response = await fetch("/api/v1/assets", { method: "POST", headers: { "Content-Type": "application/octet-stream", "X-Asset-Name": encodeURIComponent(file.name) }, body: file });
      if (!response.ok) throw new Error(await response.text());
      const asset = await response.json() as Asset;
      setAssets(previous => [...previous, { ...asset, name: file.name }]);
      if (field === "pressFrames" || field === "releaseFrames") {
        const frames = [...(previousFrames ?? current[field] ?? []), { asset: asset.id, duration: 100 }].slice(0, 32);
        update(current.id, { [field]: frames });
        return frames;
      } else update(current.id, { [field]: asset.id });
      setUploadStatus(`${file.name} assigned to ${current.part ?? "layer"}`);
    } catch (error) { setUploadStatus(error instanceof Error ? error.message : "Upload failed"); }
    return undefined;
  }
  async function importPack(file: File) {
    setUploadStatus("Importing avatar pack…");
    try {
      const response = await fetch("/api/v1/avatar-pack", { method: "POST", headers: { "Content-Type": "application/zip" }, body: file });
      if (!response.ok) throw new Error(await response.text());
      const imported = await response.json() as { name: string; part: string; id: string }[];
      if (!imported.length) throw new Error("No recognized component images. Use body/, face/, left_hand/, right_hand/, keyboard/.");
      setAssets(previous => [...previous, ...imported.map(item => ({ id: item.id, url: `/api/v1/assets/${item.id}`, name: item.name }))]);
      const layers = [...scene.layers];
      const stem = (name: string) => name.replace(/\\.(png|gif)$/i, "").toLowerCase().replace(/-/g, "_");
      for (const part of ["body", "face", "left-hand", "right-hand", "keyboard"]) {
        const sprites = imported.filter(item => item.part === part);
        if (!sprites.length) continue;
        const prefix = part.replace(/-/g, "_");
        const suffix = (name: string) => stem(name).replace(new RegExp(`^${prefix}_`), "");
        const idle = sprites.find(item => ["idle", "base"].includes(suffix(item.name)));
        if (!idle) { setUploadStatus(`Missing ${prefix}_idle.png (or ${prefix}_base.png) in avatar pack`); return; }
        const active = sprites.find(item => suffix(item.name) === "pressed");
        const released = sprites.find(item => suffix(item.name) === "released");
        const frames = (phase: "press" | "release") => sprites
          .map(item => ({ item, match: suffix(item.name).match(new RegExp(`^${phase}_(\\d+)$`)) }))
          .filter((entry): entry is { item: typeof sprites[number]; match: RegExpMatchArray } => entry.match !== null)
          .sort((left, right) => Number(left.match[1]) - Number(right.match[1]))
          .slice(0, 32).map(entry => ({ asset: entry.item.id, duration: 100 }));
        const keyPressed: Record<string, string> = {};
        if (part === "keyboard") for (const item of sprites) {
          const match = item.name.replace(/\\.(png|gif)$/i, "").match(/^(?:keyboard_)?([A-Za-z0-9_]+)_PRESSED$/i);
          if (match && !["keyboard", "base"].includes(match[1].toLowerCase())) keyPressed[match[1]] = item.id;
        }
        const existing = layers.findIndex(layer => layer.part === part);
        const base: ReactiveLayer = existing >= 0 ? layers[existing] : {
          id: `pack-${part}-${Date.now().toString(36)}`, part, idle: idle.id,
          x: 100, y: 100, width: 200, height: 200
        };
        const layer: ReactiveLayer = { ...base, idle: idle.id,
          active: part === "keyboard" ? undefined : active?.id,
          pressFrames: frames("press"),
          releaseFrames: frames("release").length ? frames("release") : released ? [{ asset: released.id, duration: 100 }] : [],
          keyPressed: part === "keyboard" ? keyPressed : undefined,
          trigger: base.trigger ?? (part === "left-hand" ? "MouseLeft" : part === "right-hand" ? "MouseRight" : undefined) };
        if (existing >= 0) layers[existing] = layer;
        else if (layers.length < 24) layers.push(layer);
      }
      onChange({ ...scene, layers });
      setUploadStatus(`Imported ${imported.length} sprites. Review component positioning and save your profile.`);
    } catch (error) { setUploadStatus(error instanceof Error ? error.message : "ZIP import failed"); }
  }
  const scene = value ?? { layers: [] };
  useEffect(() => {
    void fetch("/api/v1/assets").then(response => response.ok ? response.json() as Promise<Asset[]> : []).then(setAssets).catch(() => {});
  }, []);
  function update(id: string, change: Partial<ReactiveLayer>) {
    onChange({ ...scene, layers: scene.layers.map(layer => layer.id === id ? { ...layer, ...change } : layer) });
  }
  function add(part = "custom") {
    if (scene.layers.length >= 24) return;
    const id = `layer-${Date.now().toString(36)}`;
    onChange({ ...scene, layers: [...scene.layers, { id, part, idle: assets[0]?.id ?? "", x: 100, y: 100, width: 200, height: 200 }] });
    setSelected(id);
  }
  const current = scene.layers.find(layer => layer.id === selected);
  useEffect(() => {
    if (!animationPreview) return;
    const layer = scene.layers.find(item => item.id === animationPreview.layerId);
    const frames = layer?.[animationPreview.phase] ?? [];
    if (!frames.length) { setAnimationPreview(null); return; }
    const duration = frames[animationPreview.index]?.duration ?? 100;
    const timer = window.setTimeout(() => setAnimationPreview(previous => previous && previous.index + 1 < frames.length ? { ...previous, index: previous.index + 1 } : null), duration);
    return () => window.clearTimeout(timer);
  }, [animationPreview, value]);
  function editFrames(field: "pressFrames" | "releaseFrames", frames: ReactiveFrame[]) {
    if (current) update(current.id, { [field]: frames });
  }
  function timeline(field: "pressFrames" | "releaseFrames") {
    if (!current) return null;
    const frames = current[field] ?? [];
    return <fieldset><legend>{field === "pressFrames" ? "Press animation" : "Release animation"}</legend>
      <label>Add frames <input type="file" multiple accept="image/png,image/gif" onChange={event => { const files = Array.from(event.target.files ?? []); event.target.value = ""; void (async () => { let frames = current[field] ?? []; for (const file of files) { const next = await uploadPart(file, field, frames); if (next) frames = next; } })(); }} /></label>
      <button type="button" disabled={!frames.length} onClick={() => setAnimationPreview({ layerId: current.id, phase: field, index: 0 })}>Play {field === "pressFrames" ? "press" : "release"} frames</button>
      {frames.map((frame, index) => <div key={`${field}-${index}`} className="profile-actions">
        <img src={`/api/v1/assets/${frame.asset}`} alt={`Frame ${index + 1}`} width={48} height={48} style={{ objectFit: "contain" }} />
        <span>{assets.find(asset => asset.id === frame.asset)?.name ?? `Frame ${index + 1}`}</span>
        <label>ms <input type="number" min={20} max={5000} value={frame.duration} onChange={event => editFrames(field, frames.map((item, i) => i === index ? { ...item, duration: clamp(Number(event.target.value), 20, 5000) } : item))} /></label>
        <button type="button" disabled={index === 0} onClick={() => { const next = [...frames]; [next[index - 1], next[index]] = [next[index], next[index - 1]]; editFrames(field, next); }}>←</button>
        <button type="button" disabled={index === frames.length - 1} onClick={() => { const next = [...frames]; [next[index], next[index + 1]] = [next[index + 1], next[index]]; editFrames(field, next); }}>→</button>
        <button type="button" onClick={() => editFrames(field, frames.filter((_, i) => i !== index))}>Remove</button>
      </div>)}
    </fieldset>;
  }
  function move(id: string, delta: number) {
    const layers = [...scene.layers];
    const index = layers.findIndex(layer => layer.id === id);
    const target = index + delta;
    if (index < 0 || target < 0 || target >= layers.length) return;
    const temp = layers[index]; layers[index] = layers[target]; layers[target] = temp;
    onChange({ ...scene, layers });
  }
  
  return <section className="reactive-editor">
    <h2>Reactive scene editor</h2>
    <p>Sprite naming: COMPONENT_idle.png, COMPONENT_pressed.png, COMPONENT_released.png, COMPONENT_press_01.png, COMPONENT_release_01.png. Keyboard: keyboard_base.png and KEYNAME_PRESSED.png in keyboard/. Key names must match input event codes (for example Space or Scan_1E).</p>
    <label>Import avatar ZIP <input type="file" accept=".zip,application/zip" onChange={event => { const file = event.target.files?.[0]; if (file) void importPack(file); event.target.value = ""; }} /></label>
    <p>Use uploaded assets to create independent idle and pressed image layers. Save profiles to publish changes to OBS.</p>
    <label><input type="checkbox" checked={scene.hideDefault ?? false} onChange={event => onChange({ ...scene, hideDefault: event.target.checked })} /> Hide standard keyboard and mouse overlay</label>
    <button type="button" onClick={() => add()} disabled={scene.layers.length >= 24}>Add image layer</button>
    <div className="profile-actions">{["body", "face", "left-hand", "right-hand", "keyboard"].map(part => <button type="button" key={part} disabled={scene.layers.length >= 24} onClick={() => add(part)}>Add {part.replace("-", " ")}</button>)}</div>
    {!assets.length && <p>Add a component, then upload its images below.</p>}
    <p>Drag individual parts to position them. Select a part to edit its images, trigger, dimensions, and stacking order.</p>
    <label><input type="checkbox" checked={preview} onChange={event => setPreview(event.target.checked)} /> Preview input states</label>
    {preview && <div className="profile-actions">{["MouseLeft", "MouseRight", "Scan_1E", "Scan_20", "Space"].map(code => <button type="button" key={code} aria-pressed={held.includes(code)} onClick={() => setHeld(previous => previous.includes(code) ? previous.filter(item => item !== code) : [...previous, code])}>{code}</button>)}<button type="button" onClick={() => setHeld([])}>Release all</button></div>}
    <div style={{ position: "relative", width: 600, height: 400, maxWidth: "100%", overflow: "hidden", background: "repeating-conic-gradient(#aaa 0% 25%, #ddd 0% 50%) 50% / 20px 20px" }}>
      {scene.layers.map(layer => <img key={layer.id} src={`/api/v1/assets/${animationPreview?.layerId === layer.id ? (layer[animationPreview.phase]?.[animationPreview.index]?.asset ?? layer.idle) : layer.active && layer.trigger && triggerMatches(layer.trigger, new Set(held)) ? layer.active : layer.idle}`} alt={layer.part ?? layer.id}
        onPointerDown={event => {
          event.preventDefault();
          setSelected(layer.id);
          const target = event.currentTarget;
          target.setPointerCapture(event.pointerId);
          const startX = event.clientX, startY = event.clientY, x = layer.x, y = layer.y;
          const move = (e: PointerEvent) => update(layer.id, { x: clamp(x + e.clientX - startX, 0, 2000), y: clamp(y + e.clientY - startY, 0, 2000) });
          const end = () => { target.removeEventListener("pointermove", move); target.removeEventListener("pointerup", end); target.removeEventListener("pointercancel", end); };
          target.addEventListener("pointermove", move);
          target.addEventListener("pointerup", end, { once: true });
          target.addEventListener("pointercancel", end, { once: true });
        }}
        style={{ position: "absolute", left: layer.x, top: layer.y, width: layer.width, height: layer.height, objectFit: "contain", touchAction: "none", cursor: "move", outline: selected === layer.id ? "2px solid #6d53c5" : undefined }} />)}
    </div>
    <div className="profile-actions">{scene.layers.map(layer => <button type="button" key={layer.id} aria-pressed={selected === layer.id} onClick={() => setSelected(layer.id)}>{layer.part ?? "custom"} · {layer.id}</button>)}</div>
    {current && <div className="reactive-layer-controls">
      <button type="button" onClick={() => move(current.id, -1)}>Send backward</button>
      <button type="button" onClick={() => move(current.id, 1)}>Bring forward</button>
      <label>Part <select value={current.part ?? "custom"} onChange={event => update(current.id, { part: event.target.value })}>{["body", "face", "left-hand", "right-hand", "keyboard", "custom"].map(part => <option key={part} value={part}>{part}</option>)}</select></label>
      <label>Layer <select value={selected} onChange={event => setSelected(event.target.value)}>{scene.layers.map(layer => <option key={layer.id} value={layer.id}>{layer.id}</option>)}</select></label>
      <label>Upload idle sprite <input type="file" accept="image/png,image/gif" onChange={event => { const file = event.target.files?.[0]; if (file) void uploadPart(file, "idle"); event.target.value = ""; }} /></label>
      <label>Upload pressed sprite <input type="file" accept="image/png,image/gif" onChange={event => { const file = event.target.files?.[0]; if (file) void uploadPart(file, "active"); event.target.value = ""; }} /></label>
      <span role="status">{uploadStatus}</span>
      <label><input type="checkbox" checked={advanced} onChange={event => setAdvanced(event.target.checked)} /> Show advanced asset selectors</label>
      {advanced && <><label>Idle image <select value={current.idle} onChange={event => update(current.id, { idle: event.target.value })}>{assets.map(asset => <option key={asset.id} value={asset.id}>{asset.name ?? asset.id.slice(0, 8)}</option>)}</select></label>
      <label>Pressed image <select value={current.active ?? ""} onChange={event => update(current.id, { active: event.target.value || undefined })}><option value="">None</option>{assets.map(asset => <option key={asset.id} value={asset.id}>{asset.name ?? asset.id.slice(0, 8)}</option>)}</select></label>
      </>}
      {timeline("pressFrames")}
      {timeline("releaseFrames")}
      <label>Trigger expression <input type="text" maxLength={64} value={current.trigger ?? ""} placeholder="MouseLeft | Scan_1E+Scan_20" onChange={event => update(current.id, { trigger: event.target.value || undefined })} /></label>
      {(["x", "y", "width", "height"] as const).map(key => <label key={key}>{key} <input type="number" min={key === "width" || key === "height" ? 1 : 0} max={2000} value={current[key]} onChange={event => update(current.id, { [key]: clamp(Number(event.target.value), key === "width" || key === "height" ? 1 : 0, 2000) })} /></label>)}
      {current.trigger && <button type="button" onPointerDown={() => setHeld(current.trigger!.split("|")[0].split("+").map(code => code.trim()))} onPointerUp={() => setHeld([])} onPointerCancel={() => setHeld([])}>Hold to test trigger</button>}
      <p>Use + for simultaneous inputs, | for alternatives. Examples: MouseLeft, Scan_1E+Scan_20, MouseLeft|MouseRight.</p>
      <button type="button" onClick={() => { onChange({ ...scene, layers: scene.layers.filter(layer => layer.id !== current.id) }); setSelected(""); }}>Remove layer</button>
    </div>}
  </section>;
}
