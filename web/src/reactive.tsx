import type { ReactiveScene } from "./settings";

export function ReactiveOverlay({ scene, pressed }: { scene: ReactiveScene; pressed: string[] }) {
  const held = new Set(pressed);
  return <section aria-label="Reactive animation scene" style={{ position: "absolute", inset: 0, pointerEvents: "none" }}>
    {scene.layers.map(layer => {
      const active = Boolean(layer.active && layer.trigger && held.has(layer.trigger));
      const id = active ? layer.active : layer.idle;
      return <img key={layer.id} src={`/api/v1/assets/${id}`} alt="" draggable={false}
        style={{ position: "absolute", left: layer.x, top: layer.y, width: layer.width, height: layer.height, objectFit: "contain" }} />;
    })}
  </section>;
}
