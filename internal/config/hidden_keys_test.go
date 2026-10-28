package config

import "testing"

func TestHiddenKeysValidation(t *testing.T) {
	p := Profile{Accent: "#6d53c5", Opacity: 85, Scale: 100, Layout: "full", HiddenKeys: []string{"Ctrl", "MouseRight"}}
	if err := p.Validate(); err != nil {
		t.Fatal(err)
	}
	p.HiddenKeys = []string{"Ctrl", "Ctrl"}
	if err := p.Validate(); err == nil {
		t.Fatal("duplicate hidden key should be rejected")
	}
}

func TestHiddenKeysStoreIsolation(t *testing.T) {
	store, err := Open(t.TempDir() + "/settings.json")
	if err != nil {
		t.Fatal(err)
	}
	value := Default()
	value.Profiles = map[string]Profile{
		"demo": {Accent: "#6d53c5", Opacity: 85, Scale: 100, Layout: "full", HiddenKeys: []string{"Ctrl"}},
	}
	if err := store.Save(value); err != nil {
		t.Fatal(err)
	}
	copy := store.Get()
	copy.Profiles["demo"].HiddenKeys[0] = "Alt"
	if store.Get().Profiles["demo"].HiddenKeys[0] != "Ctrl" {
		t.Fatal("store leaked hidden key slice")
	}
}
