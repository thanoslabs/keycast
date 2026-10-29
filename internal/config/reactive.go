package config

import (
	"errors"
	"fmt"
	"regexp"
	"strings"
)

var reactiveAssetID = regexp.MustCompile("^[a-f0-9]{32}$")
var reactiveTriggerCode = regexp.MustCompile("^[A-Za-z0-9_]+$")

// ReactiveScene renders independent image layers driven by held input codes.
type ReactiveScene struct {
	HideDefault bool            `json:"hideDefault,omitempty"`
	Layers      []ReactiveLayer `json:"layers"`
}

type ReactiveFrame struct {
	Asset string `json:"asset"`
	Duration int `json:"duration"`
}

type ReactiveLayer struct {
	PressFrames []ReactiveFrame `json:"pressFrames,omitempty"`
	ReleaseFrames []ReactiveFrame `json:"releaseFrames,omitempty"`
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
		for _, frames := range [][]ReactiveFrame{l.PressFrames, l.ReleaseFrames} {
			if len(frames) > 32 { return errors.New("too many animation frames") }
			for _, frame := range frames {
				if !reactiveAssetID.MatchString(frame.Asset) || frame.Duration < 20 || frame.Duration > 5000 { return errors.New("invalid animation frame") }
			}
		}
		if len(l.Trigger) > 64 {
			return errors.New("reactive trigger too long")
		}
		if l.Trigger != "" {
			groups := strings.Split(l.Trigger, "|")
			if len(groups) > 8 {
				return errors.New("too many trigger alternatives")
			}
			for _, group := range groups {
				codes := strings.Split(group, "+")
				if len(codes) > 8 {
					return errors.New("too many simultaneous trigger codes")
				}
				for _, code := range codes {
					if !reactiveTriggerCode.MatchString(strings.TrimSpace(code)) {
						return errors.New("invalid reactive trigger expression")
					}
				}
			}
		}
		if l.X < 0 || l.Y < 0 || l.X > 2000 || l.Y > 2000 || l.Width < 1 || l.Height < 1 || l.Width > 2000 || l.Height > 2000 {
			return errors.New("reactive layer dimensions out of range")
		}
	}
	return nil
}
