import { useRef, useState, type PointerEvent } from "react";
import type { Placement } from "./settings";

export const defaultPlacement: Placement = { keyboardX: 24, keyboardY: 24, mouseX: 400, mouseY: 24 };
type Target = "keyboard" | "mouse";
const WIDTH = 960;
const HEIGHT = 540;

export function PlacementEditor({ value, onChange }: { value: Placement; onChange: (next: Placement) => void }) {
  const canvas = useRef<HTMLDivElement>(null);
  const drag = useRef<{ target: Target; startX: number; startY: number; original: Placement } | null>(null);
  const [active, setActive] = useState<Target | null>(null);

  function start(event: PointerEvent<HTMLButtonElement>, target: Target) {
    if (!canvas.current) return;
    event.currentTarget.setPointerCapture(event.pointerId);
    drag.current = { target, startX: event.clientX, startY: event.clientY, original: value };
    setActive(target);
  }

  function move(event: PointerEvent<HTMLButtonElement>) {
    const current = drag.current;
    const rect = canvas.current?.getBoundingClientRect();
    if (!current || !rect) return;
    const dx = Math.round((event.clientX - current.startX) * WIDTH / rect.width);
    const dy = Math.round((event.clientY - current.startY) * HEIGHT / rect.height);
    const xKey = current.target === "keyboard" ? "keyboardX" : "mouseX";
    const yKey = current.target === "keyboard" ? "keyboardY" : "mouseY";
    onChange({
      ...current.original,
      [xKey]: Math.max(0, Math.min(760, current.original[xKey] + dx)),
      [yKey]: Math.max(0, Math.min(450, current.original[yKey] + dy))
    });
  }

  function end() { drag.current = null; setActive(null); }

  return <div>
    <p>Drag the keyboard and mouse to position them in a 960 × 540 overlay canvas. Save the profile to apply.</p>
    <div className="placement-canvas" ref={canvas} aria-label="Overlay placement editor">
      {(["keyboard", "mouse"] as const).map(target => {
        const x = target === "keyboard" ? value.keyboardX : value.mouseX;
        const y = target === "keyboard" ? value.keyboardY : value.mouseY;
        return <button key={target} type="button"
          className={`placement-item ${active === target ? "dragging" : ""}`}
          style={{ left: `${x / WIDTH * 100}%`, top: `${y / HEIGHT * 100}%` }}
          onPointerDown={event => start(event, target)}
          onPointerMove={move}
          onPointerUp={end}
          onPointerCancel={end}
          aria-label={`Drag ${target} position`}>
          {target === "keyboard" ? "⌨ Keyboard" : "◉ Mouse"}
        </button>;
      })}
    </div>
    <button type="button" onClick={() => onChange({ ...defaultPlacement })}>Reset positions</button>
  </div>;
}
