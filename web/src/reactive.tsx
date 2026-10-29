import type { ReactiveScene } from "./settings";
import { AnimatedReactiveLayer } from "./reactive-animation";

export function ReactiveOverlay({ scene, pressed }: { scene: ReactiveScene; pressed: string[] }) {
  const held = new Set(pressed);
  return <section aria-label="Reactive animation scene" style={{ position: "absolute", inset: 0, pointerEvents: "none" }}>
    {scene.layers.map(layer => <AnimatedReactiveLayer key={layer.id} layer={layer} pressed={held} />)}
  </section>;
}
