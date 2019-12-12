package layout

import "image"

type Topology struct {
	Dimension image.Point
	Layout        Layout
}

func (t Topology) Map(x, y uint16) uint16 {
	width := uint16(t.Dimension.X)
	height := uint16(t.Dimension.Y)

	if x >= width {
		x = width - 1
	} else if x < 0 {
		x = 0
	}

	if y >= height {
		y = height - 1
	} else if y < 0 {
		y = 0
	}

	return t.Layout.Map(width, height, x, y)
}
