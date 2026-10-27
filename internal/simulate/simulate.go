package simulate

import (
	"context"
	"time"

	"github.com/thanoslabs/keycast/internal/events"
)

func Run(ctx context.Context, emit func(events.KeyEvent)) {
	steps := []events.KeyEvent{
		{Code: "ControlLeft", Action: events.Down},
		{Code: "ShiftLeft", Action: events.Down},
		{Code: "KeyK", Action: events.Down},
		{Code: "KeyK", Action: events.Up},
		{Code: "ShiftLeft", Action: events.Up},
		{Code: "ControlLeft", Action: events.Up},
	}
	ticker := time.NewTicker(450 * time.Millisecond)
	defer ticker.Stop()
	i := 0
	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			emit(steps[i])
			i = (i + 1) % len(steps)
		}
	}
}
