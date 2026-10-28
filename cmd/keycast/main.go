package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"
	"time"

	"github.com/thanoslabs/keycast/internal/analog"
	"github.com/thanoslabs/keycast/internal/capture"
	"github.com/thanoslabs/keycast/internal/config"
	"github.com/thanoslabs/keycast/internal/server"
	"github.com/thanoslabs/keycast/internal/simulate"
)

func main() {
	port := flag.Int("port", 9867, "local HTTP port")
	simulated := flag.Bool("simulate", false, "emit synthetic keyboard input")
	wooting := flag.Bool("wooting-analog", false, "enable optional Wooting Analog SDK key travel capture")
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
		log.Fatalf("cannot start Keycast on %s (another instance may already be running): %v", addr, err)
	}
	defer listener.Close()
	settings, err := config.Open(path)
	if err != nil {
		log.Fatalf("load settings: %v", err)
	}
	hub := server.New(settings)
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()
	if *wooting {
		go func() {
			if err := analog.Run(ctx, hub.EmitAnalog); err != nil { log.Printf("Wooting analog unavailable (digital input continues): %v", err) }
		}()
	}
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
	httpServer := &http.Server{Handler: hub.RoutesWithAssets(assetsDir)}
	serveErrors := make(chan error, 1)
	go func() {
		serveErrors <- httpServer.Serve(listener)
	}()
	select {
	case err := <-serveErrors:
		if err != nil && err != http.ErrServerClosed {
			log.Fatalf("server failed: %v", err)
		}
	case <-ctx.Done():
		log.Print("Keycast shutting down")
		shutdownCtx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err := httpServer.Shutdown(shutdownCtx); err != nil {
			log.Printf("graceful shutdown: %v", err)
			_ = httpServer.Close()
		}
		if err := <-serveErrors; err != nil && err != http.ErrServerClosed {
			log.Printf("server exit: %v", err)
		}
	}
}
