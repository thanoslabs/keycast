import { useRef, type CSSProperties, type PointerEvent } from "react";
import { rows } from "./layouts";

type Offset = { x: number; y: number };
type Props = {
  selected: string;
  onSelect: (key: string) => void;
  widths?: Record<string, number>;
  heights?: Record<string, number>;
  offsets?: Record<string, Offset>;
  onOffsetsChange: (offsets: Record<string, Offset>) => void;
};
const clamp = (value: number) => Math.max(-300, Math.min(300, value));

export function VisualKeyEditor({ selected, onSelect, widths = {}, heights = {}, offsets = {}, onOffsetsChange }: Props) {
  const drag = useRef<{ key: string; x: number; y: number; origin: Offset; offsets: Record<string, Offset> } | null>(null);

  function start(event: PointerEvent<HTMLButtonElement>, key: string) {
    if (event.button !== 0) return;
    onSelect(key);
    event.currentTarget.setPointerCapture(event.pointerId);
    drag.current = { key, x: event.clientX, y: event.clientY, origin: offsets[key] ?? { x: 0, y: 0 }, offsets };
  }
  function move(event: PointerEvent<HTMLButtonElement>) {
    const current = drag.current;
    if (!current) return;
    onOffsetsChange({
      ...current.offsets,
      [current.key]: {
        x: clamp(current.origin.x + Math.round(event.clientX - current.x)),
        y: clamp(current.origin.y + Math.round(event.clientY - current.y))
      }
    });
  }
  function end() { drag.current = null; }

  return <div className="visual-key-editor">
    <p>Choose a key, then drag it to adjust its position. Width and height use the controls below.</p>
    <div className="visual-key-board" aria-label="Interactive keyboard layout editor">
      {rows.map((row, index) => <div className="keyboard-row" key={index}>
        {row.map(key => <button type="button" key={key}
          aria-label={`Position ${key}`}
          aria-pressed={selected === key}
          className={`layout-key visual-key ${selected === key ? "selected" : ""}`}
          onPointerDown={event => start(event, key)}
          onPointerMove={move}
          onPointerUp={end}
          onPointerCancel={end}
          style={{
            "--key-grow": (key === "Space" ? 5 : ["Shift", "Right Shift", "Enter", "Backspace"].includes(key) ? 2 : 1) * ((widths[key] ?? 100) / 100),
            minHeight: `${35 * ((heights[key] ?? 100) / 100)}px`,
            transform: `translate(${offsets[key]?.x ?? 0}px, ${offsets[key]?.y ?? 0}px)`
          } as CSSProperties}>{key}</button>)}
      </div>)}
    </div>
  </div>;
}
