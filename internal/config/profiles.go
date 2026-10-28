package config

import (
	"errors"
	"fmt"
)

// Profile stores the appearance and layout of a named OBS browser source.
type Profile struct {
	Accent    string `json:"accent"`
	Opacity   int    `json:"opacity"`
	Scale     int    `json:"scale"`
	ShowMouse bool   `json:"showMouse"`
	Layout    string `json:"layout"`
}

func (p Profile) Validate() error {
	s := Settings{Version: 1, Accent: p.Accent, Opacity: p.Opacity, Scale: p.Scale, ShowMouse: p.ShowMouse}
	if err := s.validateAppearance(); err != nil {
		return err
	}
	if p.Layout != "compact" && p.Layout != "full" {
		return errors.New("layout must be compact or full")
	}
	return nil
}

func validateProfiles(profiles map[string]Profile) error {
	if len(profiles) > 32 {
		return errors.New("too many overlay profiles (maximum 32)")
	}
	for name, profile := range profiles {
		if len(name) == 0 || len(name) > 32 {
			return fmt.Errorf("invalid profile name %q", name)
		}
		for _, c := range name {
			if !((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
				return fmt.Errorf("invalid profile name %q: use lowercase letters, numbers and hyphens", name)
			}
		}
		if name == "default" {
			return errors.New("default is a reserved profile name")
		}
		if err := profile.Validate(); err != nil {
			return fmt.Errorf("profile %q: %w", name, err)
		}
	}
	return nil
}
