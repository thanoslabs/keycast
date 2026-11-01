import { useEffect, useState } from "react";
import type { ReactiveScene } from "./settings";

type Asset = { id: string; url: string };
type Pose = "idle" | "left" | "right" | "both";
const poses: { key: Pose; label: string; trigger?: string }[] = [
  { key: "idle", label: "Idle" },
  { key: "left", label: "Left click", trigger: "MouseLeft" },
  { key: "right", label: "Right click", trigger: "MouseRight" },
  { key: "both", label: "Both buttons", trigger: "MouseLeft+MouseRight" },
];

export function AvatarSetup({ value, onChange }: { value?: ReactiveScene; onChange: (value: ReactiveScene) => void }) {
  const [assets, setAssets] = useState<Asset[]>([]);
  const [pose, setPose] = useState<Pose>("idle");
  const [mapping, setMapping] = useState<Partial<Record<Pose, string>>>({});
  const [status, setStatus] = useState("");
  async function refresh() {
    const response = await fetch("/api/v1/assets", { cache: "no-store" });
    if (!response.ok) throw new Error("Could not load assets");
    setAssets(await response.json() as Asset[]);
  }
  useEffect(() => { void refresh().catch(() => setStatus("Asset library unavailable")); }, []);
  const selected = mapping[pose] ?? "";
  function apply() {
    if (!mapping.idle) { setStatus("Choose an idle image first."); return; }
    const layers = poses.filter(p => p.trigger && mapping[p.key]).map(p => ({
      id: `avatar-${p.key}`, idle: mapping.idle!, active: mapping[p.key],
      trigger: p.trigger, x: 100, y: 40, width: 320, height: 320,
    }));
    // A single base layer ensures idle is visible when no input is pressed.
    const base = { id: "avatar-base", idle: mapping.idle, x: 100, y: 40, width: 320, height: 320 };
    // The advanced editor remains available for fine-tuning.
    onChange({ ...value, hideDefault: true, layers: [base, ...layers] });
    setStatus("Avatar configured. Save profiles to apply it to OBS.");
  }
  return <section className="avatar-setup">
    <h2>Avatar setup</h2>
    <p>Choose your character poses, preview them, then save. For advanced animation frames and positioning, use the advanced scene editor below.</p>
    <button type="button" onClick={() => void refresh().then(() => setStatus("Assets refreshed")).catch(() => setStatus("Unable to refresh assets"))}>Refresh uploaded images</button>
    {!assets.length && <p>Upload your PNG images in Custom animation assets below, then refresh here.</p>}
    <div className="profile-actions">{poses.map(p => <button type="button" key={p.key} aria-pressed={pose === p.key} onClick={() => setPose(p.key)}>{p.label}</button>)}</div>
    <label>Image for {poses.find(p => p.key === pose)?.label}
      <select value={selected} onChange={event => setMapping(current => ({ ...current, [pose]: event.target.value }))}>
        <option value="">Choose an image</option>
        {assets.map((asset, i) => <option value={asset.id} key={asset.id}>Image {i + 1} ({asset.id.slice(0, 8)})</option>)}
      </select>
    </label>
    <div className="asset-grid">{assets.map((asset, i) => <button type="button" key={asset.id} onClick={() => setMapping(current => ({ ...current, [pose]: asset.id }))} aria-label={`Select image ${i + 1} for ${pose}`} aria-pressed={selected === asset.id}>
      <img src={asset.url} alt={`Image ${i + 1}`} style={{ width: 96, height: 96, objectFit: "contain" }} />
    </button>)}</div>
    {selected && <div><p>Preview: {pose}</p><img src={assets.find(asset => asset.id === selected)?.url} alt="Selected avatar pose" style={{ maxWidth: 240, maxHeight: 240, objectFit: "contain" }} /></div>}
    <button type="button" onClick={apply} disabled={!mapping.idle}>Use this avatar</button>
    <span role="status">{status}</span>
  </section>;
}
