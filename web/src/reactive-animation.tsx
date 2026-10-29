import { useEffect, useRef, useState } from "react";
import type { ReactiveLayer, ReactiveFrame } from "./settings";
import { triggerMatches } from "./reactive-triggers";

type Phase = "idle" | "press" | "hold" | "release";

function frameAt(frames: ReactiveFrame[], elapsed: number): string | undefined {
  let cursor = 0;
  for (const frame of frames) {
    cursor += frame.duration;
    if (elapsed < cursor) return frame.asset;
  }
  return undefined;
}

export function AnimatedReactiveLayer({ layer, pressed }: { layer: ReactiveLayer; pressed: ReadonlySet<string> }) {
  const active = triggerMatches(layer.trigger, pressed);
  const [frame, setFrame] = useState(layer.idle);
  const state = useRef<{ phase: Phase; start: number; active: boolean }>({ phase: "idle", start: 0, active: false });
  useEffect(() => {
    const now = performance.now();
    if (state.current.active !== active) {
      state.current = { phase: active ? "press" : "release", start: now, active };
    }
    const tick = () => {
      const current = state.current;
      const elapsed = performance.now() - current.start;
      const frames = current.phase === "press" ? layer.pressFrames ?? [] : current.phase === "release" ? layer.releaseFrames ?? [] : [];
      const image = frameAt(frames, elapsed);
      if (image) {
        setFrame(image);
      } else {
        if (current.phase === "press") current.phase = "hold";
        if (current.phase === "release") current.phase = "idle";
        setFrame(current.phase === "idle" ? layer.idle : layer.active || layer.idle);
      }
    };
    tick();
    const timer = window.setInterval(tick, 25);
    return () => window.clearInterval(timer);
  }, [active, layer.idle, layer.active, layer.pressFrames, layer.releaseFrames]);
  return <img src={`/api/v1/assets/${frame}`} alt="" draggable={false}
    style={{ position: "absolute", left: layer.x, top: layer.y, width: layer.width, height: layer.height, objectFit: "contain" }} />;
}
