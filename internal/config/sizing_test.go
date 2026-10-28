package config

import "testing"

func TestComponentScaleBounds(t *testing.T) {
	for _, n := range []int{0, 50, 100, 200} {
		if err := (Placement{KeyboardScale: n, MouseScale: n}).Validate(); err != nil {
			t.Fatal(err)
		}
	}
	for _, n := range []int{-1, 49, 201} {
		if (Placement{KeyboardScale: n}).Validate() == nil {
			t.Fatalf("accepted keyboard scale %d", n)
		}
		if (Placement{MouseScale: n}).Validate() == nil {
			t.Fatalf("accepted mouse scale %d", n)
		}
	}
}
