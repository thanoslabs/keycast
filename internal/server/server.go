package server

import (
	"crypto/sha1"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/thanoslabs/keycast/internal/config"
	"github.com/thanoslabs/keycast/internal/events"
	"github.com/thanoslabs/keycast/internal/state"
)

const magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

type Hub struct {
	mu       sync.Mutex
	store    *state.Store
	clients  map[chan []byte]struct{}
	settings *config.Store
}

func New(settings ...*config.Store) *Hub {
	h := &Hub{store: state.New(), clients: make(map[chan []byte]struct{})}
	if len(settings) > 0 {
		h.settings = settings[0]
	}
	return h
}

func (h *Hub) Emit(key events.KeyEvent) {
	h.mu.Lock()
	defer h.mu.Unlock()
	event, ok := h.store.Apply(key)
	if !ok {
		return
	}
	data, _ := json.Marshal(event)
	for c := range h.clients {
		select {
		case c <- data:
		default:
			close(c)
			delete(h.clients, c)
		}
	}
}

func (h *Hub) Routes() http.Handler {
	return h.RoutesWithAssets("")
}

// RoutesWithAssets serves the dashboard and OBS overlay from a packaged web build.
// An empty path keeps development/API-only behavior unchanged.
func (h *Hub) RoutesWithAssets(assetsDir string) http.Handler {
	mux := http.NewServeMux()
	mux.HandleFunc("GET /api/v1/health", func(w http.ResponseWriter, _ *http.Request) {
		w.Header().Set("Content-Type", "application/json")
		_, _ = w.Write([]byte(`{"status":"ok"}`))
	})
	mux.HandleFunc("GET /ws/v1/overlay/default", h.overlay)
	if h.settings != nil {
		settingsRoutes(mux, h.settings)
	}
	if assetsDir != "" {
		if info, err := os.Stat(filepath.Join(assetsDir, "index.html")); err == nil && !info.IsDir() {
			static := http.FileServer(http.Dir(assetsDir))
			mux.HandleFunc("GET /", func(w http.ResponseWriter, r *http.Request) {
				if r.URL.Path == "/" || r.URL.Path == "/overlay/default" {
					http.ServeFile(w, r, filepath.Join(assetsDir, "index.html"))
					return
				}
				static.ServeHTTP(w, r)
			})
		}
	}
	return mux
}

func validOrigin(origin, host string) bool {
	if origin == "" {
		return true
	}
	u, err := url.Parse(origin)
	if err != nil || u.Scheme != "http" || u.User != nil || u.Path != "" || u.RawQuery != "" || u.Fragment != "" {
		return false
	}
	if u.Host == host {
		return true
	}
	switch u.Host {
	case "localhost:5173", "127.0.0.1:5173", "localhost:5174", "127.0.0.1:5174":
		return true
	default:
		return false
	}
}

func (h *Hub) overlay(w http.ResponseWriter, r *http.Request) {
	if !validOrigin(r.Header.Get("Origin"), r.Host) {
		http.Error(w, "invalid origin", http.StatusForbidden)
		return
	}
	if !strings.EqualFold(r.Header.Get("Upgrade"), "websocket") ||
		!strings.Contains(strings.ToLower(r.Header.Get("Connection")), "upgrade") ||
		r.Header.Get("Sec-WebSocket-Version") != "13" {
		http.Error(w, "websocket upgrade required", http.StatusUpgradeRequired)
		return
	}
	key := r.Header.Get("Sec-WebSocket-Key")
	decoded, err := base64.StdEncoding.DecodeString(key)
	if err != nil || len(decoded) != 16 {
		http.Error(w, "invalid key", http.StatusBadRequest)
		return
	}
	hj, ok := w.(http.Hijacker)
	if !ok {
		http.Error(w, "hijack unavailable", http.StatusInternalServerError)
		return
	}
	conn, rw, err := hj.Hijack()
	if err != nil {
		return
	}
	defer conn.Close()
	hash := sha1.Sum([]byte(key + magic))
	if _, err = fmt.Fprintf(rw, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n", base64.StdEncoding.EncodeToString(hash[:])); err != nil {
		return
	}
	if err := rw.Flush(); err != nil {
		return
	}
	c := make(chan []byte, 32)
	h.mu.Lock()
	seq, keys := h.store.Snapshot()
	snapshot, _ := json.Marshal(struct {
		Version  int      `json:"version"`
		Type     string   `json:"type"`
		Sequence uint64   `json:"sequence"`
		Pressed  []string `json:"pressed"`
	}{1, "state.snapshot", seq, keys})
	h.clients[c] = struct{}{}
	h.mu.Unlock()
	defer func() {
		h.mu.Lock()
		if _, ok := h.clients[c]; ok {
			delete(h.clients, c)
			close(c)
		}
		h.mu.Unlock()
	}()
	if writeFrame(conn, snapshot) != nil {
		return
	}
	for data := range c {
		if writeFrame(conn, data) != nil {
			return
		}
	}
}

func writeFrame(conn net.Conn, data []byte) error {
	if err := conn.SetWriteDeadline(time.Now().Add(5 * time.Second)); err != nil {
		return err
	}
	head := []byte{0x81}
	if len(data) < 126 {
		head = append(head, byte(len(data)))
	} else {
		head = append(head, 126, byte(len(data)>>8), byte(len(data)))
	}
	_, err := conn.Write(append(head, data...))
	return err
}
