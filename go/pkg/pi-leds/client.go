package pileds

import (
	"image"

	"github.com/draeron/gopkg/color"
	"github.com/draeron/pi-leds/go/pkg/layout"
)

type Client interface {
	Close()
	SetBrightness(brightness uint8) error
	Clear(color color.Color) error
	UpdateScreen() error
	DrawImgRaw(img image.Image, mosaic layout.Mosaic) error
	DrawImg(img image.Image) error
}
