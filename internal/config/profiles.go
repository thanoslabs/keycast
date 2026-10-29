package config

import (
	"errors"
	"fmt"
	"strings"
)

// Profile stores the appearance and layout of a named OBS browser source.
type Profile struct {
	Accent      string               `json:"accent"`
	Opacity     int                  `json:"opacity"`
	Scale       int                  `json:"scale"`
	ShowMouse   bool                 `json:"showMouse"`
	Layout      string               `json:"layout"`
	Placement   *Placement           `json:"placement,omitempty"`
	KeyWidths   map[string]int       `json:"keyWidths,omitempty"`
	KeyHeights  map[string]int       `json:"keyHeights,omitempty"`
	KeyOffsets  map[string]KeyOffset `json:"keyOffsets,omitempty"`
	ReactiveScene *ReactiveScene `json:"reactiveScene,omitempty"`
	Animation   string               `json:"animation,omitempty"`
	CustomCSS   string               `json:"customCss,omitempty"`
	HiddenKeys  []string             `json:"hiddenKeys,omitempty"`
	VisibleKeys []string             `json:"visibleKeys,omitempty"`
}

type KeyOffset struct {
	X int `json:"x"`
	Y int `json:"y"`
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
	if p.ReactiveScene != nil {
		if err := p.ReactiveScene.Validate(); err != nil { return err }
	}
	if len(p.CustomCSS) > 8192 {
		return errors.New("custom CSS must be at most 8192 bytes")
	}
	lowerCSS := strings.ToLower(p.CustomCSS)
	if strings.Contains(lowerCSS, "@import") || strings.Contains(lowerCSS, "url(") || strings.Contains(lowerCSS, "expression(") || strings.Contains(lowerCSS, "</style") {
		return errors.New("custom CSS cannot load external resources or contain style tags")
	}
	if len(p.VisibleKeys) > 128 {
		return errors.New("too many visible keys")
	}
	visible := make(map[string]bool, len(p.VisibleKeys))
	for _, key := range p.VisibleKeys {
		if len(key) == 0 || len(key) > 32 || visible[key] {
			return fmt.Errorf("invalid or duplicate visible key %q", key)
		}
		visible[key] = true
	}
	if len(p.HiddenKeys) > 64 {
		return errors.New("too many hidden key labels")
	}
	seenKeys := make(map[string]bool, len(p.HiddenKeys))
	for _, key := range p.HiddenKeys {
		if len(key) == 0 || len(key) > 32 || seenKeys[key] {
			return fmt.Errorf("invalid or duplicate hidden key %q", key)
		}
		seenKeys[key] = true
	}
	if p.Animation != "" && p.Animation != "pop" && p.Animation != "glow" && p.Animation != "none" {
		return errors.New("animation must be pop, glow, or none")
	}
	if p.Layout != "compact" && p.Layout != "full" && p.Layout != "custom" {
		return errors.New("layout must be compact, full or custom")
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
	if len(p.KeyOffsets) > 128 {
		return errors.New("too many key offset overrides")
	}
	for key, offset := range p.KeyOffsets {
		if len(key) == 0 || len(key) > 32 {
			return fmt.Errorf("invalid key label %q", key)
		}
		if offset.X < -300 || offset.X > 300 || offset.Y < -300 || offset.Y > 300 {
			return fmt.Errorf("key %q offset must be between -300 and 300 pixels", key)
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
