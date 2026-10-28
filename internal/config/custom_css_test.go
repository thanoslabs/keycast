package config

import (
	"strings"
	"testing"
)

func TestProfileCustomCSSValidation(t *testing.T) {
	p := Profile{Accent: "#6d53c5", Opacity: 85, Scale: 100, Layout: "full", CustomCSS: ".overlay { color: white; }"}
	if err := p.Validate(); err != nil {
		t.Fatalf("expected valid CSS: %v", err)
	}
	for _, css := range []string{"@import 'https://example.com';", "background: url(https://example.com/a)", strings.Repeat("x", 8193)} {
		p.CustomCSS = css
		if err := p.Validate(); err == nil {
			t.Fatalf("expected invalid CSS to be rejected")
		}
	}
}
