package config

import "testing"

func TestKeyHeightValidation(t *testing.T) {
	p := Profile{Accent: "#abcdef", Opacity: 80, Scale: 100, Layout: "full", KeyHeights: map[string]int{"Space": 150}}
	if err := p.Validate(); err != nil {
		t.Fatal(err)
	}
	for _, height := range []int{49, 301} {
		p.KeyHeights["Space"] = height
		if p.Validate() == nil {
			t.Fatalf("accepted invalid key height %d", height)
		}
	}
}

func TestKeyHeightsGetIsolation(t *testing.T) {
	store := &Store{value: Default()}
	store.value.Profiles = map[string]Profile{"demo": {KeyHeights: map[string]int{"Space": 150}}}
	value := store.Get()
	value.Profiles["demo"].KeyHeights["Space"] = 200
	if store.Get().Profiles["demo"].KeyHeights["Space"] != 150 {
		t.Fatal("key height map leaked from store")
	}
}
