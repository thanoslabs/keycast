import React from "react";
import { createRoot } from "react-dom/client";
import "./style.css";

function App() {
  const overlay = window.location.pathname.startsWith("/overlay");
  return overlay ? (
    <main className="overlay" aria-label="Keycast overlay preview">
      <span className="key">Ctrl</span><span className="key">Shift</span><span className="key">K</span>
    </main>
  ) : (
    <main className="dashboard">
      <h1>Keycast</h1>
      <p>Windows-first keyboard and mouse overlay for OBS.</p>
      <p>Foundation preview only — live capture and WebSocket streaming are not yet implemented.</p>
      <a href="/overlay/default">Open static overlay preview</a>
    </main>
  );
}

createRoot(document.getElementById("root")!).render(<React.StrictMode><App /></React.StrictMode>);
