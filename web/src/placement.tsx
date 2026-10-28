import { useRef, useState, type KeyboardEvent, type PointerEvent } from "react";
import type { Placement } from "./settings";

export const defaultPlacement: Placement = { keyboardX: 24, keyboardY: 24, mouseX: 400, mouseY: 24 };
type Target = "keyboard" | "mouse";
const WIDTH = 960;
const HEIGHT = 540;

export function PlacementEditor({ value, onChange }: { value: Placement; onChange: (next: Placement) => void }) {
  const canvas = useRef<HTMLDivElement>(null);
  const drag = useRef<{ target: Target; startX: number; startY: number; original: Placement } | null>(null);
  const [active, setActive] = useState<Target | null>(null);
  const [grid, setGrid] = useState(8);
  const [snapEnabled, setSnapEnabled] = useState(true);
  const [guides, setGuides] = useState(true);
  const step = snapEnabled ? grid : 1;
  const adjust = (n: number, max: number) => Math.max(0, Math.min(max, Math.round(n / step) * step));

  function start(event: PointerEvent<HTMLButtonElement>, target: Target) {
    if (event.button !== 0 || !canvas.current) return;
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
      [xKey]: adjust(current.original[xKey] + dx, 760),
      [yKey]: adjust(current.original[yKey] + dy, 450)
    });
  }

  function end() { drag.current = null; setActive(null); }

  function nudge(event: KeyboardEvent<HTMLButtonElement>, target: Target) {
    const directions: Record<string, [string, number]> = {
      ArrowLeft: ["X", -1], ArrowRight: ["X", 1], ArrowUp: ["Y", -1], ArrowDown: ["Y", 1]
    };
    const direction = directions[event.key];
    if (!direction) return;
    event.preventDefault();
    const [axis, sign] = direction;
    const key = `${target}${axis}` as keyof Placement;
    const amount = event.shiftKey ? 10 : step;
    onChange({ ...value, [key]: adjust(value[key] + sign * amount, axis === "X" ? 760 : 450) });
  }

  return <div>
    <p>Drag the keyboard and mouse to position them in a 960 × 540 overlay canvas. Save the profile to apply.</p>
    <div className="placement-options">
      <label><input type="checkbox" checked={snapEnabled} onChange={event => setSnapEnabled(event.target.checked)} /> Snap to grid</label>
      <label>Grid size <select value={grid} disabled={!snapEnabled} onChange={event => setGrid(Number(event.target.value))}>
        <option value={4}>4 px</option><option value={8}>8 px</option><option value={16}>16 px</option><option value={32}>32 px</option>
      </select></label>
      <label><input type="checkbox" checked={guides} onChange={event => setGuides(event.target.checked)} /> Show grid</label>
    </div>
    <div className="placement-canvas" ref={canvas} aria-label="Overlay placement editor" data-guides={guides} style={{ backgroundSize: `${grid / WIDTH * 100}% ${grid / HEIGHT * 100}%` }}>
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
          onKeyDown={event => nudge(event, target)}
          aria-label={`Drag ${target} position`}>
          {target === "keyboard" ? "⌨ Keyboard" : "◉ Mouse"}
        </button>;
      })}
    </div>
    <div className="placement-coordinates">
      {(["keyboard", "mouse"] as const).map(target => <fieldset key={target}>
        <legend>{target} position (px)</legend>
        {(["X", "Y"] as const).map(axis => {
          const key = `${target}${axis}` as keyof Placement;
          return <label key={axis}>{axis} <input type="number" min={0} max={axis === "X" ? 760 : 450}
            value={value[key]} onChange={event => {
              const n = Number(event.target.value);
              if (Number.isFinite(n)) onChange({ ...value, [key]: adjust(n, axis === "X" ? 760 : 450) });
            }} /></label>;
        })}
      </fieldset>)}
    </div>
    <button type="button" onClick={() => onChange({ ...defaultPlacement })}>Reset positions</button>
  </div>;
}
