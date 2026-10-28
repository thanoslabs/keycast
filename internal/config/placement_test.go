package config

import "testing"

func TestPlacementValidation(t *testing.T) {
	valid := Profile{Accent: "#abcdef", Opacity: 80, Scale: 100, Layout: "compact", Placement: &Placement{KeyboardX: 0, KeyboardY: 540, MouseX: 900, MouseY: 0}}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	invalid := valid
	invalid.Placement = &Placement{KeyboardX: -1}
	if invalid.Validate() == nil {
		t.Fatal("negative position was accepted")
	}
	invalid.Placement = &Placement{MouseX: 2001}
	if invalid.Validate() == nil {
		t.Fatal("oversized position was accepted")
	}
}

func TestPlacementCopyIsolation(t *testing.T) {
	store := &Store{value: Default()}
	settings := store.Get()
	settings.Profiles = map[string]Profile{"test": {Accent: "#abcdef", Opacity: 80, Scale: 100, Layout: "full", Placement: &Placement{KeyboardX: 20}}}
	// Save is exercised by the persistence tests; this test checks that Get returns a deep copy.
	store.value = settings
	copied := store.Get()
	copied.Profiles["test"].Placement.KeyboardX = 900
	if store.Get().Profiles["test"].Placement.KeyboardX != 20 {
		t.Fatal("Get exposed internal placement pointer")
	}
}
