package server

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"errors"
	"image"
	_ "image/gif"
	_ "image/png"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/thanoslabs/keycast/internal/config"
)

const maxAssetBytes = 4 << 20
const maxAssets = 64

var validAssetID = regexp.MustCompile("^[a-f0-9]{32}$")

type assetInfo struct {
	ID  string `json:"id"`
	URL string `json:"url"`
	Name string `json:"name,omitempty"`
}

func assetRoutes(mux *http.ServeMux, store *config.Store) {
	root := filepath.Join(filepath.Dir(store.Path()), "assets")
	mux.HandleFunc("GET /api/v1/assets", func(w http.ResponseWriter, r *http.Request) {
		entries, err := os.ReadDir(root)
		if errors.Is(err, os.ErrNotExist) {
			entries = nil
			err = nil
		}
		if err != nil {
			http.Error(w, "unable to list assets", 500)
			return
		}
		items := make([]assetInfo, 0)
		for _, entry := range entries {
			if entry.IsDir() {
				continue
			}
			name := entry.Name()
			ext := filepath.Ext(name)
			id := name[:len(name)-len(ext)]
			if validAssetID.MatchString(id) && (ext == ".png" || ext == ".gif") {
				displayName := id + ext
				if data, readErr := os.ReadFile(filepath.Join(root, id+".name")); readErr == nil { displayName = string(data) }
				items = append(items, assetInfo{ID: id, URL: "/api/v1/assets/" + id, Name: displayName})
			}
		}
		w.Header().Set("Content-Type", "application/json")
		w.Header().Set("Cache-Control", "no-store")
		_ = json.NewEncoder(w).Encode(items)
	})
	mux.HandleFunc("DELETE /api/v1/assets/{id}", func(w http.ResponseWriter, r *http.Request) {
		if !validOrigin(r.Header.Get("Origin"), r.Host) {
			http.Error(w, "invalid origin", 403)
			return
		}
		id := r.PathValue("id")
		if !validAssetID.MatchString(id) {
			http.NotFound(w, r)
			return
		}
		for _, ext := range []string{".png", ".gif"} {
			err := os.Remove(filepath.Join(root, id+ext))
			if err == nil {
				_ = os.Remove(filepath.Join(root, id+".name"))
				w.WriteHeader(http.StatusNoContent)
				return
			}
			if !errors.Is(err, os.ErrNotExist) {
				http.Error(w, "unable to delete asset", 500)
				return
			}
		}
		http.NotFound(w, r)
	})
	mux.HandleFunc("POST /api/v1/assets", func(w http.ResponseWriter, r *http.Request) {
		if !validOrigin(r.Header.Get("Origin"), r.Host) {
			http.Error(w, "invalid origin", http.StatusForbidden)
			return
		}
		entries, err := os.ReadDir(root)
		if err != nil && !errors.Is(err, os.ErrNotExist) {
			http.Error(w, "unable to list assets", 500)
			return
		}
		count := 0
		for _, entry := range entries {
			if strings.HasSuffix(entry.Name(), ".png") || strings.HasSuffix(entry.Name(), ".gif") {
				count++
			} }
		if count >= maxAssets {
			http.Error(w, "asset limit reached", 400)
			return
		}
		data, err := io.ReadAll(io.LimitReader(r.Body, maxAssetBytes+1))
		if err != nil || len(data) == 0 || len(data) > maxAssetBytes {
			http.Error(w, "invalid image size", 400)
			return
		}
		mime := http.DetectContentType(data)
		ext := ""
		switch mime {
		case "image/png":
			ext = ".png"
		case "image/gif":
			ext = ".gif"
		}
		if ext == "" {
			http.Error(w, "PNG or GIF required", 415)
			return
		}
		cfg, _, err := image.DecodeConfig(bytes.NewReader(data))
		if err != nil || cfg.Width < 1 || cfg.Height < 1 || cfg.Width > 4096 || cfg.Height > 4096 {
			http.Error(w, "invalid image", 400)
			return
		}
		if err = os.MkdirAll(root, 0700); err != nil {
			http.Error(w, "storage unavailable", 500)
			return
		}
		var idBytes [16]byte
		if _, err = rand.Read(idBytes[:]); err != nil {
			http.Error(w, "storage unavailable", 500)
			return
		}
		id := hex.EncodeToString(idBytes[:])
		if err = os.WriteFile(filepath.Join(root, id+ext), data, 0600); err != nil {
			http.Error(w, "storage unavailable", 500)
			return
		}
		rawName, decodeErr := url.QueryUnescape(r.Header.Get("X-Asset-Name"))
		if decodeErr != nil {
			rawName = ""
		}
		name := filepath.Base(strings.ReplaceAll(rawName, "\\", "/"))
		if name == "." || name == "" {
			name = id + ext
		}
		if len(name) > 128 {
			name = name[:128]
		}
		_ = os.WriteFile(filepath.Join(root, id+".name"), []byte(name), 0600)
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusCreated)
		_ = json.NewEncoder(w).Encode(assetInfo{ID: id, URL: "/api/v1/assets/" + id, Name: name})
	})
	mux.HandleFunc("GET /api/v1/assets/{id}", func(w http.ResponseWriter, r *http.Request) {
		id := r.PathValue("id")
		if !validAssetID.MatchString(id) {
			http.NotFound(w, r)
			return
		}
		for _, ext := range []string{".png", ".gif"} {
			path := filepath.Join(root, id+ext)
			if _, err := os.Stat(path); err == nil {
				w.Header().Set("X-Content-Type-Options", "nosniff")
				http.ServeFile(w, r, path)
				return
			}
		}
		http.NotFound(w, r)
	})
}
