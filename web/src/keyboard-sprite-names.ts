// Human-friendly keyboard sprite filenames mapped to Keycast's Windows scan-code events.
// Keep the canonical event code as the stored key so live input and preview agree.
export const keyboardNameMap: ReadonlyArray<readonly [string, string]> = [
  ["1", "Scan_02"], ["2", "Scan_03"], ["3", "Scan_04"], ["4", "Scan_05"], ["5", "Scan_06"],
  ["Tab", "Scan_0F"], ["Q", "Scan_10"], ["W", "Scan_11"], ["E", "Scan_12"], ["R", "Scan_13"], ["T", "Scan_14"],
  ["Caps", "Scan_3A"], ["CapsLock", "Scan_3A"],
  ["A", "Scan_1E"], ["S", "Scan_1F"], ["D", "Scan_20"], ["F", "Scan_21"], ["G", "Scan_22"],
  ["Shift", "Scan_2A"], ["LShift", "Scan_2A"],
  ["Z", "Scan_2C"], ["X", "Scan_2D"], ["C", "Scan_2E"], ["V", "Scan_2F"],
  ["Ctrl", "Scan_1D"], ["LCtrl", "Scan_1D"],
  ["Alt", "Scan_38"], ["LAlt", "Scan_38"], ["Space", "Space"],
];

export function keyboardSpriteCode(name: string): string | undefined {
  const normalized = name.trim().toLowerCase();
  if (/^scan_[0-9a-f]{2}$/i.test(name)) return `Scan_${name.slice(5).toUpperCase()}`;
  if (normalized === "space") return "Space";
  return keyboardNameMap.find(([label]) => label.toLowerCase() === normalized)?.[1];
}
