package config

import "testing"

func TestKeyWidthValidation(t *testing.T) {
	profile := Profile{Accent: "#abcdef", Opacity: 80, Scale: 100, Layout: "full", KeyWidths: map[string]int{"Space": 150}}
	if err := profile.Validate(); err != nil {
		t.Fatal(err)
	}
	profile.KeyWidths["Space"] = 301
	if profile.Validate() == nil {
		t.Fatal("accepted oversized key width")
	}
	profile.KeyWidths["Space"] = 49
	if profile.Validate() == nil {
		t.Fatal("accepted undersized key width")
	}
}

func TestKeyWidthsGetIsolation(t *testing.T) {
	store := &Store{value: Default()}
	store.value.Profiles = map[string]Profile{"demo": {KeyWidths: map[string]int{"Space": 150}}}
	value := store.Get()
	value.Profiles["demo"].KeyWidths["Space"] = 200
	if store.Get().Profiles["demo"].KeyWidths["Space"] != 150 {
		t.Fatal("key width map leaked from store")
	}
}
