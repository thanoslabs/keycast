package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
)

func main() {
	port := flag.Int("port", 9867, "local HTTP port")
	flag.Parse()

	mux := http.NewServeMux()
	mux.HandleFunc("GET /api/v1/health", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "application/json")
		_, _ = w.Write([]byte(`{"status":"ok"}`))
	})
	addr := fmt.Sprintf("127.0.0.1:%d", *port)
	log.Printf("Keycast development server listening on http://%s", addr)
	log.Fatal(http.ListenAndServe(addr, mux))
}
