package server

import (
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRoutesWithAssets(t *testing.T) {
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "index.html"), []byte("<html>Keycast dashboard</html>"), 0600); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(filepath.Join(dir, "assets"), 0700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "assets", "test.js"), []byte("console.log('keycast')"), 0600); err != nil {
		t.Fatal(err)
	}
	handler := New().RoutesWithAssets(dir)
	for _, path := range []string{"/", "/overlay/default", "/assets/test.js"} {
		recorder := httptest.NewRecorder()
		handler.ServeHTTP(recorder, httptest.NewRequest(http.MethodGet, path, nil))
		if recorder.Code != http.StatusOK {
			t.Fatalf("%s: status %d", path, recorder.Code)
		}
	}
	recorder := httptest.NewRecorder()
	handler.ServeHTTP(recorder, httptest.NewRequest(http.MethodGet, "/api/v1/health", nil))
	if recorder.Code != http.StatusOK || !strings.Contains(recorder.Body.String(), "ok") {
		t.Fatalf("health endpoint broken: %d %s", recorder.Code, recorder.Body.String())
	}
}
