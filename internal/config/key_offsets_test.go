package config

import "testing"

func TestKeyOffsetValidation(t *testing.T) {
	p := Profile{Accent: "#abcdef", Opacity: 80, Scale: 100, Layout: "full", KeyOffsets: map[string]KeyOffset{"Space": {X: -300, Y: 300}}}
	if err := p.Validate(); err != nil {
		t.Fatal(err)
	}
	p.KeyOffsets["Space"] = KeyOffset{X: 301}
	if p.Validate() == nil {
		t.Fatal("accepted offset beyond limit")
	}
}

func TestKeyOffsetsGetIsolation(t *testing.T) {
	store := &Store{value: Default()}
	store.value.Profiles = map[string]Profile{"demo": {KeyOffsets: map[string]KeyOffset{"Space": {X: 12}}}}
	value := store.Get()
	value.Profiles["demo"].KeyOffsets["Space"] = KeyOffset{X: 200}
	if store.Get().Profiles["demo"].KeyOffsets["Space"].X != 12 {
		t.Fatal("key offsets leaked from store")
	}
}
