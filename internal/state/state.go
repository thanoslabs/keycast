package state

import (
	"sort"
	"sync"

	"github.com/thanoslabs/keycast/internal/events"
)

type Store struct {
	mu       sync.RWMutex
	pressed  map[string]bool
	sequence uint64
}

func New() *Store { return &Store{pressed: make(map[string]bool)} }

func (s *Store) Apply(key events.KeyEvent) (events.Envelope, bool) {
	s.mu.Lock()
	defer s.mu.Unlock()
	if key.Code == "" || (key.Action != events.Down && key.Action != events.Up) {
		return events.Envelope{}, false
	}
	was := s.pressed[key.Code]
	if key.Action == events.Down {
		s.pressed[key.Code] = true
	} else {
		delete(s.pressed, key.Code)
	}
	if was == (key.Action == events.Down) {
		return events.Envelope{}, false
	}
	s.sequence++
	return events.Envelope{Version: 1, Type: "input.key", Sequence: s.sequence, Data: key}, true
}

func (s *Store) Snapshot() (uint64, []string) {
	s.mu.RLock()
	defer s.mu.RUnlock()
	keys := make([]string, 0, len(s.pressed))
	for key := range s.pressed {
		keys = append(keys, key)
	}
	sort.Strings(keys)
	return s.sequence, keys
}
