import React, { useEffect, useState } from "react";
import { createRoot } from "react-dom/client";
import "./style.css";
import { SettingsDashboard, defaultSettings, loadSettings, type Settings } from "./settings";
import { KeyboardLayout, parseLayout } from "./layouts";
import { defaultPlacement } from "./placement";

type Message = { version: number; type: string; sequence: number; pressed?: string[]; data?: { code: string; action: "down" | "up" } };
const mouseCodes = ["MouseLeft", "MouseMiddle", "MouseRight", "MouseButton4", "MouseButton5"];
const wheels: Record<string, string> = { WheelUp: "↑", WheelDown: "↓", WheelLeft: "←", WheelRight: "→" };
const scans: Record<string, string> = {
  "01":"Esc","02":"1","03":"2","04":"3","05":"4","06":"5","07":"6","08":"7","09":"8","0A":"9","0B":"0",
  "0E":"Backspace","0F":"Tab","10":"Q","11":"W","12":"E","13":"R","14":"T","15":"Y","16":"U","17":"I",
  "18":"O","19":"P","1C":"Enter","1D":"Ctrl","1E":"A","1F":"S","20":"D","21":"F","22":"G","23":"H",
  "24":"J","25":"K","26":"L","2A":"Shift","2C":"Z","2D":"X","2E":"C","2F":"V","30":"B","31":"N",
  "32":"M","36":"Right Shift","38":"Alt","39":"Space","3B":"F1","3C":"F2","3D":"F3","3E":"F4",
  "3F":"F5","40":"F6","41":"F7","42":"F8","43":"F9","44":"F10","57":"F11","58":"F12"
};
const extended: Record<string, string> = {
  "1C":"Num Enter","1D":"Right Ctrl","38":"Right Alt","47":"Home","48":"↑","49":"Page Up",
  "4B":"←","4D":"→","4F":"End","50":"↓","51":"Page Down","52":"Insert","53":"Delete",
  "5B":"Left Win","5C":"Right Win"
};
function label(code: string): string {
  const e = /^ScanE0_([0-9A-F]+)$/.exec(code);
  if (e) return extended[e[1]] ?? code;
  const s = /^Scan_([0-9A-F]+)$/.exec(code);
  if (s) return scans[s[1]] ?? code;
  if (code === "ControlLeft") return "Ctrl";
  if (code === "ShiftLeft") return "Shift";
  if (/^Key[A-Z]$/.test(code)) return code.slice(3);
  return code;
}
function Mouse({ pressed, wheel }: { pressed: string[]; wheel: string | null }) {
  const down = new Set(pressed);
  return <section className="mouse-panel" aria-label="Mouse input">
    <div className="mouse-shape" aria-hidden="true">
      <div className={`mouse-button left ${down.has("MouseLeft") ? "active" : ""}`} />
      <div className={`mouse-button right ${down.has("MouseRight") ? "active" : ""}`} />
      <div className={`mouse-wheel ${down.has("MouseMiddle") ? "active" : ""}`} />
      <div className={`mouse-side side-one ${down.has("MouseButton4") ? "active" : ""}`} />
      <div className={`mouse-side side-two ${down.has("MouseButton5") ? "active" : ""}`} />
    </div>
    <div className="mouse-wheel-status">{wheel ? `Scroll ${wheels[wheel]}` : "Mouse"}</div>
  </section>;
}
function Overlay() {
  const [pressed, setPressed] = useState<string[]>([]);
  const [wheel, setWheel] = useState<string | null>(null);
  const [connected, setConnected] = useState(false);
  const [settings, setSettings] = useState<Settings>(defaultSettings);
  useEffect(() => {
    let active = true;
    const refresh = () => { void loadSettings().then(value => { if (active) setSettings(value); }).catch(() => {}); };
    refresh();
    const interval = setInterval(refresh, 2000);
    return () => { active = false; clearInterval(interval); };
  }, []);
  useEffect(() => {
    let active = true;
    let socket: WebSocket | undefined;
    let retry: ReturnType<typeof setTimeout> | undefined;
    let wheelTimeout: ReturnType<typeof setTimeout> | undefined;
    let sequence = -1;
    function connect() {
      if (!active) return;
      const url = new URL("/ws/v1/overlay/default", window.location.href);
      url.protocol = url.protocol === "https:" ? "wss:" : "ws:";
      socket = new WebSocket(url);
      socket.onopen = () => setConnected(true);
      socket.onmessage = event => {
        try {
          const message = JSON.parse(event.data) as Message;
          if (message.version !== 1 || !Number.isSafeInteger(message.sequence)) return;
          if (message.type === "state.snapshot") {
            sequence = message.sequence;
            setPressed(Array.isArray(message.pressed) ? message.pressed : []);
            setWheel(null);
            return;
          }
          if (message.type !== "input.key" || !message.data || message.sequence <= sequence) return;
          if (sequence >= 0 && message.sequence !== sequence + 1) {
            socket?.close();
            return;
          }
          sequence = message.sequence;
          const { code, action } = message.data;
          if (code in wheels) {
            if (action === "down") {
              setWheel(code);
              clearTimeout(wheelTimeout);
              wheelTimeout = setTimeout(() => setWheel(null), 380);
            }
            return;
          }
          if (action === "down") setPressed(previous => [...new Set([...previous, code])]);
          if (action === "up") setPressed(previous => previous.filter(key => key !== code));
        } catch { /* Ignore malformed messages. */ }
      };
      socket.onclose = () => {
        setConnected(false);
        setPressed([]);
        setWheel(null);
        clearTimeout(wheelTimeout);
        if (active) retry = setTimeout(connect, 1000);
      };
      socket.onerror = () => socket?.close();
    }
    connect();
    return () => {
      active = false;
      clearTimeout(retry);
      clearTimeout(wheelTimeout);
      socket?.close();
    };
  }, []);
  const keys = pressed.filter(code => !mouseCodes.includes(code) && !(code in wheels)).map(label);
  const params = new URLSearchParams(window.location.search);
  const profileName = params.get("profile") ?? "default";
  const profile = profileName === "default" ? undefined : settings.profiles?.[profileName];
  const appearance = profile ?? settings;
  const layout = profile ? profile.layout : parseLayout(params.get("layout"));
  const placement = profile?.placement;
  const positions = placement ?? defaultPlacement;
  return <main className={`overlay ${placement ? "overlay-positioned" : ""}`} aria-label="Live Keycast input overlay" style={{ transform: `scale(${appearance.scale / 100})`, transformOrigin: "top left", opacity: appearance.opacity / 100, ["--accent" as string]: appearance.accent }}>
    <section className={`keyboard-panel ${layout === "full" ? "keyboard-panel-full" : ""}`} style={placement ? { position: "absolute", left: positions.keyboardX, top: positions.keyboardY, transform: `scale(${(positions.keyboardScale || 100) / 100})`, transformOrigin: "top left" } : undefined} aria-label="Pressed keyboard keys">
      <KeyboardLayout layout={layout} pressed={keys} keyWidths={profile?.keyWidths} keyHeights={profile?.keyHeights} />
    </section>
    {appearance.showMouse && (placement ? <div style={{ position: "absolute", left: positions.mouseX, top: positions.mouseY, transform: `scale(${(positions.mouseScale || 100) / 100})`, transformOrigin: "top left" }}><Mouse pressed={pressed} wheel={wheel} /></div> : <Mouse pressed={pressed} wheel={wheel} />)}
    {!connected && <div className="connection" role="status">Waiting for Keycast…</div>}
  </main>;
}
function App() {
  return window.location.pathname.startsWith("/overlay") ? <Overlay /> : <SettingsDashboard />;
}
createRoot(document.getElementById("root")!).render(<React.StrictMode><App /></React.StrictMode>);
