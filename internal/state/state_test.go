package state

import (
	"testing"

	"github.com/thanoslabs/keycast/internal/events"
)

func TestTransitions(t *testing.T) {
	s := New()
	if _, ok := s.Apply(events.KeyEvent{Code: "KeyA", Action: events.Down}); !ok {
		t.Fatal("missing down")
	}
	if _, ok := s.Apply(events.KeyEvent{Code: "KeyA", Action: events.Down}); ok {
		t.Fatal("duplicate down")
	}
	if _, ok := s.Apply(events.KeyEvent{Code: "KeyA", Action: events.Up}); !ok {
		t.Fatal("missing up")
	}
	_, keys := s.Snapshot()
	if len(keys) != 0 {
		t.Fatalf("stuck keys %v", keys)
	}
}
