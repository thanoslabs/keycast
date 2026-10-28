package config

import (
	"path/filepath"
	"reflect"
	"testing"
)

func TestSaveAndReload(t *testing.T) {
	path := filepath.Join(t.TempDir(), "settings.json")
	store, err := Open(path)
	if err != nil {
		t.Fatal(err)
	}
	value := store.Get()
	value.Accent = "#abcdef"
	value.Scale = 125
	if err := store.Save(value); err != nil {
		t.Fatal(err)
	}
	reloaded, err := Open(path)
	if err != nil {
		t.Fatal(err)
	}
	if !reflect.DeepEqual(reloaded.Get(), value) {
		t.Fatalf("reloaded settings differ: %+v", reloaded.Get())
	}
}

func TestRejectInvalidSettings(t *testing.T) {
	for _, value := range []Settings{
		{Version: 2, Accent: "#abcdef", Opacity: 80, Scale: 100},
		{Version: 1, Accent: "#nothex", Opacity: 80, Scale: 100},
		{Version: 1, Accent: "#abcdef", Opacity: 101, Scale: 100},
	} {
		if value.Validate() == nil {
			t.Fatalf("accepted invalid settings: %+v", value)
		}
	}
}
