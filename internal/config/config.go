package config

import (
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"sync"
)

type Settings struct {
	Version   int                `json:"version"`
	Accent    string             `json:"accent"`
	Opacity   int                `json:"opacity"`
	Scale     int                `json:"scale"`
	ShowMouse bool               `json:"showMouse"`
	Profiles  map[string]Profile `json:"profiles,omitempty"`
}

func Default() Settings {
	return Settings{Version: 1, Accent: "#6d53c5", Opacity: 85, Scale: 100, ShowMouse: true}
}

func (s Settings) Validate() error {
	if s.Version != 1 {
		return errors.New("unsupported settings version")
	}
	if err := s.validateAppearance(); err != nil {
		return err
	}
	return validateProfiles(s.Profiles)
}

func (s Settings) validateAppearance() error {
	if len(s.Accent) != 7 || s.Accent[0] != '#' {
		return errors.New("accent must be a six-digit hex color")
	}
	for _, c := range s.Accent[1:] {
		if !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
			return errors.New("accent must be a six-digit hex color")
		}
	}
	if s.Opacity < 20 || s.Opacity > 100 || s.Scale < 50 || s.Scale > 200 {
		return errors.New("opacity or scale outside allowed range")
	}
	return nil
}

type Store struct {
	mu    sync.RWMutex
	path  string
	value Settings
}

func Open(path string) (*Store, error) {
	store := &Store{path: path, value: Default()}
	data, err := os.ReadFile(path)
	if errors.Is(err, os.ErrNotExist) {
		return store, nil
	}
	if err != nil {
		return nil, err
	}
	if err := json.Unmarshal(data, &store.value); err != nil {
		return nil, err
	}
	if err := store.value.Validate(); err != nil {
		return nil, err
	}
	return store, nil
}

func (s *Store) Get() Settings {
	s.mu.RLock()
	defer s.mu.RUnlock()
	value := s.value
	if value.Profiles != nil {
		value.Profiles = make(map[string]Profile, len(s.value.Profiles))
		for name, profile := range s.value.Profiles {
			profile.HiddenKeys = append([]string(nil), profile.HiddenKeys...)
			profile.VisibleKeys = append([]string(nil), profile.VisibleKeys...)
			if profile.Placement != nil {
				placement := *profile.Placement
				profile.Placement = &placement
			}
			if profile.KeyOffsets != nil {
				offsets := make(map[string]KeyOffset, len(profile.KeyOffsets))
				for key, offset := range profile.KeyOffsets {
					offsets[key] = offset
				}
				profile.KeyOffsets = offsets
			}
			if profile.KeyHeights != nil {
				heights := make(map[string]int, len(profile.KeyHeights))
				for key, height := range profile.KeyHeights {
					heights[key] = height
				}
				profile.KeyHeights = heights
			}
			if profile.KeyWidths != nil {
				widths := make(map[string]int, len(profile.KeyWidths))
				for key, width := range profile.KeyWidths {
					widths[key] = width
				}
				profile.KeyWidths = widths
			}
			value.Profiles[name] = profile
		}
	}
	return value
}

func (s *Store) Save(value Settings) error {
	if err := value.Validate(); err != nil {
		return err
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if err := os.MkdirAll(filepath.Dir(s.path), 0700); err != nil {
		return err
	}
	data, err := json.MarshalIndent(value, "", "  ")
	if err != nil {
		return err
	}
	tmp, err := os.CreateTemp(filepath.Dir(s.path), ".settings-*.tmp")
	if err != nil {
		return err
	}
	defer os.Remove(tmp.Name())
	if err := tmp.Chmod(0600); err != nil {
		tmp.Close()
		return err
	}
	if _, err := tmp.Write(data); err != nil {
		tmp.Close()
		return err
	}
	if err := tmp.Close(); err != nil {
		return err
	}
	if err := os.Rename(tmp.Name(), s.path); err != nil {
		return err
	}
	s.value = value
	if value.Profiles != nil {
		s.value.Profiles = make(map[string]Profile, len(value.Profiles))
		for name, profile := range value.Profiles {
			profile.HiddenKeys = append([]string(nil), profile.HiddenKeys...)
			profile.VisibleKeys = append([]string(nil), profile.VisibleKeys...)
			if profile.Placement != nil {
				placement := *profile.Placement
				profile.Placement = &placement
			}
			if profile.KeyOffsets != nil {
				offsets := make(map[string]KeyOffset, len(profile.KeyOffsets))
				for key, offset := range profile.KeyOffsets {
					offsets[key] = offset
				}
				profile.KeyOffsets = offsets
			}
			if profile.KeyHeights != nil {
				heights := make(map[string]int, len(profile.KeyHeights))
				for key, height := range profile.KeyHeights {
					heights[key] = height
				}
				profile.KeyHeights = heights
			}
			if profile.KeyWidths != nil {
				widths := make(map[string]int, len(profile.KeyWidths))
				for key, width := range profile.KeyWidths {
					widths[key] = width
				}
				profile.KeyWidths = widths
			}
			s.value.Profiles[name] = profile
		}
	}
	return nil
}
