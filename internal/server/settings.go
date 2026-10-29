package server

import (
	"encoding/json"
	"io"
	"net/http"

	"github.com/thanoslabs/keycast/internal/config"
)

func settingsRoutes(mux *http.ServeMux, store *config.Store) {
	mux.HandleFunc("GET /api/v1/settings", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "application/json")
		w.Header().Set("Cache-Control", "no-store")
		_ = json.NewEncoder(w).Encode(store.Get())
	})
	mux.HandleFunc("PUT /api/v1/settings", func(w http.ResponseWriter, r *http.Request) {
		if !validOrigin(r.Header.Get("Origin"), r.Host) {
			http.Error(w, "invalid origin", http.StatusForbidden)
			return
		}
		if r.Header.Get("Content-Type") != "application/json" {
			http.Error(w, "JSON required", http.StatusUnsupportedMediaType)
			return
		}
		var value config.Settings
		dec := json.NewDecoder(io.LimitReader(r.Body, 262145))
		dec.DisallowUnknownFields()
		if err := dec.Decode(&value); err != nil {
			http.Error(w, "invalid JSON settings", http.StatusBadRequest)
			return
		}
		var extra any
		if err := dec.Decode(&extra); err != io.EOF {
			http.Error(w, "unexpected JSON content", http.StatusBadRequest)
			return
		}
		if err := value.Validate(); err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		if err := store.Save(value); err != nil {
			http.Error(w, "unable to persist settings", http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Type", "application/json")
		_ = json.NewEncoder(w).Encode(value)
	})
}
