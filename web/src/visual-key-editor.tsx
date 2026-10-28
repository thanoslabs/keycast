import { useRef, useState, type CSSProperties, type KeyboardEvent, type PointerEvent } from "react";
import { rows } from "./layouts";

type Offset = { x: number; y: number };
type Props = {
  selected: string;
  onSelect: (key: string) => void;
  widths?: Record<string, number>;
  heights?: Record<string, number>;
  offsets?: Record<string, Offset>;
  onOffsetsChange: (offsets: Record<string, Offset>) => void;
  onWidthsChange: (widths: Record<string, number>) => void;
  onHeightsChange: (heights: Record<string, number>) => void;
};
const clamp = (value: number) => Math.max(-300, Math.min(300, value));
const size = (value: number) => Math.max(50, Math.min(300, value));
export const snap = (value: number, grid: number) => Math.round(value / grid) * grid;

export function VisualKeyEditor({ selected, onSelect, widths = {}, heights = {}, offsets = {}, onOffsetsChange, onWidthsChange, onHeightsChange }: Props) {
  const drag = useRef<{ key: string; x: number; y: number; origin: Offset; offsets: Record<string, Offset>; mode: "move" | "width" | "height"; originSize: number; originalSizes: Record<string, number> } | null>(null);
  const [grid, setGrid] = useState(8);
  const [snapping, setSnapping] = useState(true);
  const [showOverlap, setShowOverlap] = useState(true);
  const [overlaps, setOverlaps] = useState<string[]>([]);
  const apply = (value: number) => clamp(snapping ? snap(value, grid) : value);

  function start(event: PointerEvent<HTMLElement>, key: string, mode: "move" | "width" | "height" = "move") {
    if (event.button !== 0) return;
    onSelect(key);
    event.currentTarget.setPointerCapture(event.pointerId);
    drag.current = { key, x: event.clientX, y: event.clientY, origin: offsets[key] ?? { x: 0, y: 0 }, offsets, mode,
      originSize: mode === "width" ? widths[key] ?? 100 : heights[key] ?? 100,
      originalSizes: mode === "width" ? widths : heights };
  }
  function move(event: PointerEvent<HTMLButtonElement>) {
    const current = drag.current;
    if (!current) return;
    if (current.mode !== "move") {
      const delta = current.mode === "width" ? event.clientX - current.x : event.clientY - current.y;
      const next = size(Math.round((current.originSize + delta * 2) / 5) * 5);
      const sizes = { ...current.originalSizes, [current.key]: next };
      if (current.mode === "width") onWidthsChange(sizes);
      else onHeightsChange(sizes);
      return;
    }
    onOffsetsChange({
      ...current.offsets,
      [current.key]: {
        x: apply(current.origin.x + Math.round(event.clientX - current.x)),
        y: apply(current.origin.y + Math.round(event.clientY - current.y))
      }
    });
  }
  function end() { drag.current = null; }
  function nudge(event: KeyboardEvent<HTMLButtonElement>, key: string) {
    const direction: Record<string, Offset> = {
      ArrowLeft: { x: -1, y: 0 }, ArrowRight: { x: 1, y: 0 },
      ArrowUp: { x: 0, y: -1 }, ArrowDown: { x: 0, y: 1 }
    };
    const delta = direction[event.key];
    if (!delta) return;
    event.preventDefault();
    const previous = offsets[key] ?? { x: 0, y: 0 };
    const step = event.shiftKey ? 10 : 1;
    onSelect(key);
    onOffsetsChange({ ...offsets, [key]: { x: clamp(previous.x + delta.x * step), y: clamp(previous.y + delta.y * step) } });
  }

  function checkOverlaps() {
    const buttons = document.querySelectorAll<HTMLButtonElement>(".visual-key-board .visual-key");
    const rects = Array.from(buttons).map(button => ({ key: button.dataset.key ?? "", rect: button.getBoundingClientRect() }));
    const hits = new Set<string>();
    for (let i = 0; i < rects.length; i++) for (let j = i + 1; j < rects.length; j++) {
      const a = rects[i].rect, b = rects[j].rect;
      if (a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top) {
        hits.add(rects[i].key); hits.add(rects[j].key);
      }
    }
    setOverlaps([...hits]);
  }

  return <div className="visual-key-editor">
    <p>Drag keys to reposition them, or drag a key's right/bottom handle to resize. Arrow keys nudge by 1px; Shift + arrows by 10px.</p>
    <div className="visual-editor-options">
      <label><input type="checkbox" checked={snapping} onChange={event => setSnapping(event.target.checked)} /> Snap to grid</label>
      <label>Grid <select value={grid} onChange={event => setGrid(Number(event.target.value))}>
        {[4, 8, 16, 32].map(n => <option value={n} key={n}>{n}px</option>)}
      </select></label>
      <label><input type="checkbox" checked={showOverlap} onChange={event => setShowOverlap(event.target.checked)} /> Show overlaps</label>
      <button type="button" onClick={checkOverlaps}>Check overlaps</button>
    </div>
    <div className="visual-key-board" aria-label="Interactive keyboard layout editor">
      {rows.map((row, index) => <div className="keyboard-row" key={index}>
        {row.map(key => <button type="button" key={key} data-key={key}
          aria-label={`Position ${key}`}
          aria-pressed={selected === key}
          className={`layout-key visual-key ${selected === key ? "selected" : ""} ${showOverlap && overlaps.includes(key) ? "overlapping" : ""}`}
          onPointerDown={event => start(event, key)}
          onPointerMove={move}
          onPointerUp={end}
          onPointerCancel={end}
          onKeyDown={event => nudge(event, key)}
          style={{
            "--key-grow": (key === "Space" ? 5 : ["Shift", "Right Shift", "Enter", "Backspace"].includes(key) ? 2 : 1) * ((widths[key] ?? 100) / 100),
            minHeight: `${35 * ((heights[key] ?? 100) / 100)}px`,
            transform: `translate(${offsets[key]?.x ?? 0}px, ${offsets[key]?.y ?? 0}px)`
          } as CSSProperties}>{key}
          {selected === key && <>
            <span className="key-resize-handle width" aria-hidden="true"
              onPointerDown={event => { event.stopPropagation(); start(event, key, "width"); }} />
            <span className="key-resize-handle height" aria-hidden="true"
              onPointerDown={event => { event.stopPropagation(); start(event, key, "height"); }} />
          </>}
        </button>)}
      </div>)}
    </div>
    {showOverlap && overlaps.length > 0 && <p role="status">{overlaps.length} keys overlap. Adjust their positions or sizes.</p>}
  </div>;
}
