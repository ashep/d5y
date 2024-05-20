package main

import (
	"github.com/ashep/go-apprun"

	"github.com/ashep/d5y/internal/app"
)

func main() {
	apprun.Run(app.New, app.Config{})
}
