package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"net"
	"errors"
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
	addr := fmt.Sprintf("127.0.0.1:%d", *port)
	listener, err := net.Listen("tcp", addr)
	if err != nil {
		var opErr *net.OpError
		if errors.As(err, &opErr) {
			log.Fatalf("cannot start Keycast on %s (another instance may already be running): %v", addr, err)
		}
		log.Fatalf("cannot listen on %s: %v", addr, err)
	}
	defer listener.Close()
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
	assetsDir := ""
	if exe, err := os.Executable(); err == nil {
		assetsDir = filepath.Join(filepath.Dir(exe), "web")
	}
	log.Printf("Keycast listening on http://%s", addr)
	log.Fatal(http.Serve(listener, hub.RoutesWithAssets(assetsDir)))
}
