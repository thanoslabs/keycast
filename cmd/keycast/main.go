package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"

	"github.com/thanoslabs/keycast/internal/capture"
	"github.com/thanoslabs/keycast/internal/config"
	"github.com/thanoslabs/keycast/internal/server"
	"github.com/thanoslabs/keycast/internal/simulate"
)

func main() {
	port := flag.Int("port", 9867, "local HTTP port")
	simulated := flag.Bool("simulate", false, "emit synthetic keyboard input")
	configPath := flag.String("config", "", "settings file path (defaults to OS user config directory)")
	flag.Parse()
	path := *configPath
	if path == "" {
		dir, err := os.UserConfigDir()
		if err != nil {
			log.Fatal(err)
		}
		path = filepath.Join(dir, "Keycast", "settings.json")
	}
	settings, err := config.Open(path)
	if err != nil {
		log.Fatalf("load settings: %v", err)
	}
	hub := server.New(settings)
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	if *simulated {
		go simulate.Run(ctx, hub.Emit)
	} else {
		go func() {
			if err := capture.Run(ctx, hub.Emit); err != nil {
				log.Printf("native input capture unavailable: %v", err)
			}
		}()
	}
	addr := fmt.Sprintf("127.0.0.1:%d", *port)
	log.Printf("Keycast listening on http://%s", addr)
	log.Fatal(http.ListenAndServe(addr, hub.Routes()))
}
