import { useEffect, useState } from "react";
import type { ReactiveLayer, ReactiveScene } from "./settings";

type Asset = { id: string; url: string };
type Props = { value?: ReactiveScene; onChange: (scene: ReactiveScene) => void };
const clamp = (value: number, min: number, max: number) => Math.max(min, Math.min(max, Math.round(value) || min));

export function ReactiveSceneEditor({ value, onChange }: Props) {
  const [assets, setAssets] = useState<Asset[]>([]);
  const [selected, setSelected] = useState("");
  const [held, setHeld] = useState<string[]>([]);
  const scene = value ?? { layers: [] };
  useEffect(() => {
    void fetch("/api/v1/assets").then(response => response.ok ? response.json() as Promise<Asset[]> : []).then(setAssets).catch(() => {});
  }, []);
  function update(id: string, change: Partial<ReactiveLayer>) {
    onChange({ ...scene, layers: scene.layers.map(layer => layer.id === id ? { ...layer, ...change } : layer) });
  }
  function add() {
    if (!assets.length || scene.layers.length >= 24) return;
    const id = `layer-${Date.now().toString(36)}`;
    onChange({ ...scene, layers: [...scene.layers, { id, idle: assets[0].id, x: 100, y: 100, width: 200, height: 200 }] });
    setSelected(id);
  }
  const current = scene.layers.find(layer => layer.id === selected);
  const options = ["", "MouseLeft", "MouseRight", "MouseMiddle", "MouseButton4", "MouseButton5", "Space", "Enter", "Scan_1E", "Scan_20", "Scan_39"];
  return <section className="reactive-editor">
    <h2>Reactive scene editor</h2>
    <p>Use uploaded assets to create independent idle and pressed image layers. Save profiles to publish changes to OBS.</p>
    <label><input type="checkbox" checked={scene.hideDefault ?? false} onChange={event => onChange({ ...scene, hideDefault: event.target.checked })} /> Hide standard keyboard and mouse overlay</label>
    <button type="button" onClick={add} disabled={!assets.length || scene.layers.length >= 24}>Add image layer</button>
    {!assets.length && <p>Upload a PNG or GIF in the asset library below to get started.</p>}
    <div style={{ position: "relative", width: 600, height: 400, maxWidth: "100%", overflow: "hidden", background: "repeating-conic-gradient(#aaa 0% 25%, #ddd 0% 50%) 50% / 20px 20px" }}>
      {scene.layers.map(layer => <img key={layer.id} src={`/api/v1/assets/${layer.active && layer.trigger && held.includes(layer.trigger) ? layer.active : layer.idle}`} alt={layer.id}
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
    {current && <div className="reactive-layer-controls">
      <label>Layer <select value={selected} onChange={event => setSelected(event.target.value)}>{scene.layers.map(layer => <option key={layer.id} value={layer.id}>{layer.id}</option>)}</select></label>
      <label>Idle image <select value={current.idle} onChange={event => update(current.id, { idle: event.target.value })}>{assets.map(asset => <option key={asset.id} value={asset.id}>{asset.id.slice(0, 8)}</option>)}</select></label>
      <label>Pressed image <select value={current.active ?? ""} onChange={event => update(current.id, { active: event.target.value || undefined })}><option value="">None</option>{assets.map(asset => <option key={asset.id} value={asset.id}>{asset.id.slice(0, 8)}</option>)}</select></label>
      <label>Trigger <select value={current.trigger ?? ""} onChange={event => update(current.id, { trigger: event.target.value || undefined })}>{options.map(code => <option key={code} value={code}>{code || "None"}</option>)}</select></label>
      {(["x", "y", "width", "height"] as const).map(key => <label key={key}>{key} <input type="number" min={key === "width" || key === "height" ? 1 : 0} max={2000} value={current[key]} onChange={event => update(current.id, { [key]: clamp(Number(event.target.value), key === "width" || key === "height" ? 1 : 0, 2000) })} /></label>)}
      {current.trigger && <button type="button" onPointerDown={() => setHeld([current.trigger!])} onPointerUp={() => setHeld([])} onPointerCancel={() => setHeld([])}>Hold to test trigger</button>}
      <button type="button" onClick={() => { onChange({ ...scene, layers: scene.layers.filter(layer => layer.id !== current.id) }); setSelected(""); }}>Remove layer</button>
    </div>}
  </section>;
}
