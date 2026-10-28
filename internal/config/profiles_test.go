package config

import (
	"path/filepath"
	"testing"
)

func TestProfilesPersistAndIsolate(t *testing.T) {
	store, err := Open(filepath.Join(t.TempDir(), "settings.json"))
	if err != nil {
		t.Fatal(err)
	}
	settings := store.Get()
	settings.Profiles = map[string]Profile{"stream": {Accent: "#123abc", Opacity: 80, Scale: 125, ShowMouse: true, Layout: "full"}}
	if err := store.Save(settings); err != nil {
		t.Fatal(err)
	}
	copy := store.Get()
	copy.Profiles["stream"] = Profile{}
	if store.Get().Profiles["stream"].Layout != "full" {
		t.Fatal("Get exposed mutable map")
	}
	reopened, err := Open(store.path)
	if err != nil {
		t.Fatal(err)
	}
	if reopened.Get().Profiles["stream"].Accent != "#123abc" {
		t.Fatal("profile was not persisted")
	}
}

func TestProfileValidation(t *testing.T) {
	for _, name := range []string{"default", "UPPER", "with spaces", ""} {
		s := Default()
		s.Profiles = map[string]Profile{name: {Accent: "#123abc", Opacity: 80, Scale: 100, Layout: "full"}}
		if s.Validate() == nil {
			t.Fatalf("accepted invalid profile name %q", name)
		}
	}
	s := Default()
	s.Profiles = map[string]Profile{"stream": {Accent: "bad", Opacity: 80, Scale: 100, Layout: "full"}}
	if s.Validate() == nil {
		t.Fatal("accepted invalid profile appearance")
	}
}
