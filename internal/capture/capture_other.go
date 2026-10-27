//go:build !windows

package capture

import (
	"context"
	"errors"

	"github.com/thanoslabs/keycast/internal/events"
)

func Run(_ context.Context, _ func(events.KeyEvent)) error {
	return errors.New("native input capture is supported only on Windows")
}
