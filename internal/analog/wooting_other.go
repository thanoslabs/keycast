//go:build !windows

package analog

import (
	"context"
	"errors"
)

type Sample struct {
	Code  string  `json:"code"`
	Depth float32 `json:"depth"`
}

func Run(_ context.Context, _ func([]Sample)) error {
	return errors.New("Wooting analog capture requires Windows")
}
