/** Keyboard Layout Editor raw-data geometry (https://www.keyboard-layout-editor.com/).
 * Accepts the JSON array form from KLE's Raw Data tab, not its serialized metadata object.
 * KLE uses persistent w/h until overridden, and x/y as relative offsets.
 */
export type KleKey = { label: string; x: number; y: number; w: number; h: number; rotation: number; rotationX: number; rotationY: number };
export function parseKleLayout(input: string): KleKey[] {
  if (input.length > 100_000) throw new Error("KLE layout exceeds 100 KB");
  const raw: unknown = JSON.parse(input);
  if (!Array.isArray(raw)) throw new Error("Expected a KLE raw-data array");
  const keys: KleKey[] = [];
  let y = 0;
  let rotation = 0, rotationX = 0, rotationY = 0;
  for (const row of raw) {
    if (!Array.isArray(row)) continue; // KLE metadata object
    let x = 0, w = 1, h = 1;
    let nextX = 0, nextY = 0;
    for (const entry of row) {
      if (typeof entry === "object" && entry !== null && !Array.isArray(entry)) {
        const props = entry as Record<string, unknown>;
        const number = (name: string, fallback: number) => {
          const value = props[name];
          if (value === undefined) return fallback;
          if (typeof value !== "number" || !Number.isFinite(value) || Math.abs(value) > 100) throw new Error(`Invalid KLE ${name}`);
          return value;
        };
        if ("rx" in props) { rotationX = number("rx", rotationX); x = rotationX; }
        if ("ry" in props) { rotationY = number("ry", rotationY); y = rotationY; }
        rotation = number("r", rotation);
        nextX += number("x", 0);
        nextY += number("y", 0);
        w = number("w", w);
        h = number("h", h);
        if (w <= 0 || h <= 0) throw new Error("Invalid KLE key dimensions");
      } else if (typeof entry === "string") {
        x += nextX;
        const label = entry.split("\\n")[0];
        keys.push({ label, x, y: y + nextY, w, h, rotation, rotationX, rotationY });
        if (keys.length > 256) throw new Error("Too many keys");
        x += w;
        nextX = 0; nextY = 0; w = 1; h = 1;
      }
    }
    y += 1;
  }
  if (!keys.length) throw new Error("No keys found in KLE layout");
  return keys;
}
