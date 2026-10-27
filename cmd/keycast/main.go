package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"

	"github.com/thanoslabs/keycast/internal/server"
	"github.com/thanoslabs/keycast/internal/simulate"
)

func main() {
	port := flag.Int("port", 9867, "local HTTP port")
	simulated := flag.Bool("simulate", false, "emit synthetic keyboard input")
	flag.Parse()
	hub := server.New()
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	if *simulated {
		go simulate.Run(ctx, hub.Emit)
	}
	addr := fmt.Sprintf("127.0.0.1:%d", *port)
	log.Printf("Keycast listening on http://%s", addr)
	log.Fatal(http.ListenAndServe(addr, hub.Routes()))
}
