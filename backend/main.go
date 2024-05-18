// Author:  Oleksandr Shepetko
// Email:   a@shepetko.com
// License: MIT

package main

import (
	"context"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/ashep/d5y/api"
)

func main() {
	wAPIKey := os.Getenv("WEATHER_API_KEY")
	if wAPIKey == "" {
		panic("WEATHER_API_KEY environment variable is not set")
	}

	s := api.New(":9000", wAPIKey)
	s.Run()

	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)
	sig := <-sigCh
	log.Printf("signal received: %v", sig)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*15)
	s.Shutdown(ctx)
	cancel()
}
