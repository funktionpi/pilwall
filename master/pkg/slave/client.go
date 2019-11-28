package slave

import (
  "image"
  "led-wall/pkg/color"
  "led-wall/pkg/layout"
)

type Client interface {
  Close()
  SetBrightness(brightness uint8) error
  Clear(color color.Color) error
  GetDimension() (*DimensionResponse, error)
  UpdateScreen() error
  DrawLine(x1, y1, x2, y2 int16, rgba color.Color) error
  DrawImgRaw(img image.Image, mosaic layout.Mosaic) error
  DrawImg(img image.Image) error
}

