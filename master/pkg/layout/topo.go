package layout

type Topology struct {
	Width, Height uint16
	Layout        LayoutFunc
}

func (t Topology) Map(x, y uint16) uint16 {
	if x >= t.Width {
		x = t.Width - 1
	} else if x < 0 {
		x = 0
	}

	if y >= t.Height {
		y = t.Height - 1
	} else if y < 0 {
		y = 0
	}

	return t.Layout(t.Width, t.Height, x, y)
}
