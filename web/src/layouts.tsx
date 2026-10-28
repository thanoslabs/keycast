import type { CSSProperties } from "react";

export type Layout = "compact" | "full";
export const layoutOptions: Layout[] = ["compact", "full"];

export function parseLayout(value: string | null): Layout {
  return value === "full" ? "full" : "compact";
}

export const rows = [
  ["Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"],
  ["`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace"],
  ["Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\"],
  ["Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "Enter"],
  ["Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "Right Shift"],
  ["Ctrl", "Win", "Alt", "Space", "Right Alt", "Right Win", "Menu", "Right Ctrl"]
];

const aliases: Record<string, string> = {
  "Right Ctrl": "Ctrl", "Right Alt": "Alt", "Right Win": "Win",
  "Left Win": "Win", "Right Shift": "Shift"
};

export function KeyboardLayout({ layout, pressed, keyWidths = {} }: { layout: Layout; pressed: string[]; keyWidths?: Record<string, number> }) {
  if (layout === "compact") {
    return <div className="key-list">
      {pressed.map(key => <span className="key" key={key}>{key}</span>)}
      {pressed.length === 0 && <span className="empty-keys" aria-hidden="true">Keyboard</span>}
    </div>;
  }
  const active = new Set(pressed.map(key => aliases[key] ?? key));
  return <div className="full-keyboard" aria-label="Full keyboard layout">
    {rows.map((row, index) => <div className="keyboard-row" key={index}>
      {row.map(key => <span key={key}
        className={`layout-key ${active.has(key) ? "is-pressed" : ""}`}
        style={{ "--key-grow": (key === "Space" ? 5 : ["Shift", "Right Shift", "Enter", "Backspace"].includes(key) ? 2 : 1) * ((keyWidths[key] ?? 100) / 100) } as CSSProperties}
      >{key}</span>)}
    </div>)}
  </div>;
}
