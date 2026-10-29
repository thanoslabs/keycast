package config

import (
	"errors"
	"fmt"
	"regexp"
)

var reactiveAssetID = regexp.MustCompile("^[a-f0-9]{32}$")

// ReactiveScene renders independent image layers driven by held input codes.
type ReactiveScene struct {
	HideDefault bool            `json:"hideDefault,omitempty"`
	Layers      []ReactiveLayer `json:"layers"`
}

type ReactiveLayer struct {
	ID      string `json:"id"`
	Idle    string `json:"idle"`
	Active  string `json:"active,omitempty"`
	Trigger string `json:"trigger,omitempty"`
	X       int    `json:"x"`
	Y       int    `json:"y"`
	Width   int    `json:"width"`
	Height  int    `json:"height"`
}

func (s ReactiveScene) Validate() error {
	if len(s.Layers) > 24 {
		return errors.New("too many reactive layers")
	}
	seen := make(map[string]bool)
	for _, l := range s.Layers {
		if len(l.ID) < 1 || len(l.ID) > 32 || seen[l.ID] {
			return fmt.Errorf("invalid reactive layer ID %q", l.ID)
		}
		seen[l.ID] = true
		if !reactiveAssetID.MatchString(l.Idle) {
			return fmt.Errorf("invalid idle asset for layer %q", l.ID)
		}
		if l.Active != "" && !reactiveAssetID.MatchString(l.Active) {
			return fmt.Errorf("invalid active asset for layer %q", l.ID)
		}
		if len(l.Trigger) > 64 {
			return errors.New("reactive trigger too long")
		}
		if l.X < 0 || l.Y < 0 || l.X > 2000 || l.Y > 2000 || l.Width < 1 || l.Height < 1 || l.Width > 2000 || l.Height > 2000 {
			return errors.New("reactive layer dimensions out of range")
		}
	}
	return nil
}
