package main

import (
	"github.com/aykevl/ledsgo/demos"
	"github.com/dlsniper/debugger"
	"github.com/draeron/gopkg/errors"
	"github.com/funktionpi/pilwall/go/pkg/discovery"
	"github.com/funktionpi/pilwall/go/pkg/layout"
	"github.com/funktionpi/pilwall/go/pkg/pi-leds"
	"image"
	"image/color"
	"image/draw"
	"time"
)

func main() {

	var dnses []discovery.DnsEntry

	debugger.SetLabels(func() []string {
		return []string{"routine", "main"}
	})

	for len(dnses) == 0 {
		dnses = discovery.LookupZeroconf()
	}

	svr := dnses[0]

	mosaic := layout.Mosaic{
		TileSize: svr.TileCount,
		TileTopo: layout.MustParseLayout(svr.TileLayout),
		PanelTopo: layout.Topology{
			Dimension: svr.TileDimension,
			Layout:    layout.MustParseLayout(svr.PanelLayout),
		},
	}

	client, err := pileds.ConnectProto(svr.Ip, svr.Port)
	errors.ExitIfErr(err)

	tick := time.NewTicker(time.Second / 60)

	img := image.NewRGBA(image.Rect(0,0, int(mosaic.Width()), int(mosaic.Height())))
	display := &display{
		img:    img,
		cl: client,
		Mosaic: mosaic,
	}

	client.SetBrightness(255)

	for range tick.C {
		demos.Fire(display, time.Now())
		display.Draw()
	}
}


type display struct {
	img draw.Image
	cl pileds.Client
	layout.Mosaic
}

func (d *display) Draw() {
	d.cl.DrawImg(d.img)
	d.cl.UpdateScreen()
}
func (d *display) Size() (x, y int16) {
	return int16(d.Mosaic.Width()), int16(d.Mosaic.Height())
}

func (d *display) SetPixel(x, y int16, c color.RGBA) {
	d.img.Set(int(x), int(y), c)
}
