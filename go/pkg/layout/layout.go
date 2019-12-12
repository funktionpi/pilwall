package layout

import (
	"bytes"
	"image"

	"github.com/draeron/gopkg/color"
)

type Mosaic struct {
	TileSize  image.Point
	TileTopo  Layout
	PanelTopo Topology
}

/*
  Give a coordinate, tell which index the LED is
*/
func (t Mosaic) Map(x, y uint16) uint16 {
	totalWidth := t.Width()
	totalHeight := t.Height()

	if x >= totalWidth {
		x = totalWidth - 1
	} else if x < 0 {
		x = 0
	}

	if y >= totalHeight {
		y = totalHeight - 1
	} else if y < 0 {
		y = 0
	}

	var localIndex uint16
	var tileOffset uint16

	t.calculate(x, y, &localIndex, &tileOffset)
	return localIndex + tileOffset
}

func (t Mosaic) Width() uint16 {
	return uint16(t.PanelTopo.Dimension.X * t.TileSize.X)
}

func (t Mosaic) Height() uint16 {
	return uint16(t.PanelTopo.Dimension.Y * t.TileSize.Y)
}

func (t Mosaic) Count() uint16 {
	return t.Width() * t.Height()
}

func (t Mosaic) calculate(x, y uint16, pLocalIndex *uint16, pTileOffset *uint16) {
	width := uint16(t.PanelTopo.Dimension.X)
	height := uint16(t.PanelTopo.Dimension.Y)

	tileX := x / width
	topoX := x % width

	tileY := y / height
	topoY := y % height

	*pTileOffset = t.TileTopo.Map(uint16(t.TileSize.X), uint16(t.TileSize.Y), tileX, tileY) * width * height
	*pLocalIndex = t.PanelTopo.Map(topoX, topoY)
}

/*
  Slice an image into a slice of pixels
*/
func (t Mosaic) Slice(img image.Image) []color.Color {
	size := img.Bounds().Size()
	out := make([]color.Color, size.X*size.Y)

	for x := img.Bounds().Min.X; x < img.Bounds().Max.X; x++ {
		for y := img.Bounds().Min.Y; y < img.Bounds().Max.Y; y++ {
			col := color.FromColor(img.At(x, y))
			// recalibrate to 0 based coord
			x -= img.Bounds().Min.X
			y -= img.Bounds().Min.Y
			idx := t.Map(uint16(x), uint16(y))
			out[idx] = col
		}
	}
	return out
}

/*
  Transform an image into a serie of bytes
*/
func (t Mosaic) SliceToBytes(img image.Image) []byte {
	buf := bytes.Buffer{}
	for _, col := range t.Slice(img) {
		rgb := col.RGB()
		buf.WriteByte(rgb.R)
		buf.WriteByte(rgb.G)
		buf.WriteByte(rgb.B)
	}
	return buf.Bytes()
}
