package pileds

import (
	"image"

	"github.com/draeron/pi-leds/go/pkg/api/pi-proto"
	"github.com/draeron/pi-leds/go/pkg/layout"
	"github.com/draeron/gopkg/color"
)

type Client interface {
	Close()
	SetBrightness(brightness uint8) error
	Clear(color color.Color) error
	GetDimension() (*pi_proto.DimensionResponse, error)
	UpdateScreen() error
	DrawLine(x1, y1, x2, y2 int16, rgba color.Color) error
	DrawImgRaw(img image.Image, mosaic layout.Mosaic) error
	DrawImg(img image.Image) error
}
