package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"
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
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
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
	addr := fmt.Sprintf("127.0.0.1:%d", *port)
	log.Printf("Keycast listening on http://%s", addr)
	listener := &http.Server{Addr: addr, Handler: hub.RoutesWithAssets(assetsDir)}
	serveErrors := make(chan error, 1)
	go func() { serveErrors <- listener.ListenAndServe() }()
	select {
	case err := <-serveErrors:
		if err != nil && err != http.ErrServerClosed {
			log.Fatalf("server failed: %v", err)
		}
	case <-ctx.Done():
		log.Print("Keycast shutting down")
		shutdownCtx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err := listener.Shutdown(shutdownCtx); err != nil {
			log.Printf("graceful shutdown: %v", err)
			_ = listener.Close()
		}
		if err := <-serveErrors; err != nil && err != http.ErrServerClosed {
			log.Printf("server exit: %v", err)
		}
	}
}
