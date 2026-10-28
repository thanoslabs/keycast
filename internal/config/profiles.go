package config

import (
	"errors"
	"fmt"
)

// Profile stores the appearance and layout of a named OBS browser source.
type Profile struct {
	Accent     string         `json:"accent"`
	Opacity    int            `json:"opacity"`
	Scale      int            `json:"scale"`
	ShowMouse  bool           `json:"showMouse"`
	Layout     string         `json:"layout"`
	Placement  *Placement     `json:"placement,omitempty"`
	KeyWidths  map[string]int `json:"keyWidths,omitempty"`
	KeyHeights map[string]int `json:"keyHeights,omitempty"`
}

type Placement struct {
	KeyboardX     int `json:"keyboardX"`
	KeyboardY     int `json:"keyboardY"`
	MouseX        int `json:"mouseX"`
	MouseY        int `json:"mouseY"`
	KeyboardScale int `json:"keyboardScale,omitempty"`
	MouseScale    int `json:"mouseScale,omitempty"`
}

func (p Placement) Validate() error {
	for _, value := range []int{p.KeyboardX, p.KeyboardY, p.MouseX, p.MouseY} {
		if value < 0 || value > 2000 {
			return errors.New("overlay position must be between 0 and 2000")
		}
	}
	for _, scale := range []int{p.KeyboardScale, p.MouseScale} {
		if scale != 0 && (scale < 50 || scale > 200) {
			return errors.New("component scale must be between 50 and 200 percent")
		}
	}
	return nil
}

func (p Profile) Validate() error {
	s := Settings{Version: 1, Accent: p.Accent, Opacity: p.Opacity, Scale: p.Scale, ShowMouse: p.ShowMouse}
	if err := s.validateAppearance(); err != nil {
		return err
	}
	if p.Layout != "compact" && p.Layout != "full" {
		return errors.New("layout must be compact or full")
	}
	if p.Placement != nil {
		if err := p.Placement.Validate(); err != nil {
			return err
		}
	}
	if len(p.KeyWidths) > 128 {
		return errors.New("too many key width overrides")
	}
	for key, width := range p.KeyWidths {
		if len(key) == 0 || len(key) > 32 {
			return fmt.Errorf("invalid key label %q", key)
		}
		if width < 50 || width > 300 {
			return fmt.Errorf("key %q width must be 50 to 300 percent", key)
		}
	}
	if len(p.KeyHeights) > 128 {
		return errors.New("too many key height overrides")
	}
	for key, height := range p.KeyHeights {
		if len(key) == 0 || len(key) > 32 {
			return fmt.Errorf("invalid key label %q", key)
		}
		if height < 50 || height > 300 {
			return fmt.Errorf("key %q height must be 50 to 300 percent", key)
		}
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
