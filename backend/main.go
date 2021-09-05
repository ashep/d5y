// Author:  Alexander Shepetko
// Email:   a@shepetko.com
// License: MIT

package main

import (
	"context"
	"github.com/ashep/cronus/service"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"
)

func main() {
	wApiKey := os.Getenv("WEATHER_API_KEY")
	if wApiKey == "" {
		panic("WEATHER_API_KEY environment variable is not set")
	}

	s := service.New(":9000", wApiKey)
	s.Run()

	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
	sig := <-sigs
	log.Printf("signal received: %v", sig)

	ctx, cancel := context.WithTimeout(context.Background(), time.Second * 15)
	s.Shutdown(ctx)
	cancel()
}
