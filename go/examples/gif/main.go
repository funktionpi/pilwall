package main

import (
	"fmt"
	"github/draeron/pileds/go/pkg/api/pi-proto"
	"image/gif"
	"github/draeron/pileds/go/pkg/discovery"
	"github/draeron/pileds/go/pkg/layout"
	"github/draeron/pileds/go/pkg/pi-leds"
	"net"
	"os"
	"os/signal"
	"sync"
	"time"

	"github.com/draeron/gopkg/color"
	"github.com/draeron/gopkg/errors"
	"github.com/fogleman/gg"
	"github.com/golang/protobuf/proto"
)

func main() {

	println("looking for mDNS services... ")

	var dnses []discovery.DnsEntry

	for len(dnses) == 0 {
		dnses = discovery.LookupZeroconf()
	}

	svr := dnses[0]

	mosaic := layout.Mosaic{
		TileSize: svr.TileCount,
		TileTopo: layout.MustParseLayout(svr.TileLayout),
		PanelTopo: layout.Topology{
			Dimension: svr.PanelDimension,
			Layout:    layout.MustParseLayout(svr.PanelLayout),
		},
	}
	mosaic.Height()

	client, err := pi_leds.ConnectProto(svr.Ip, svr.Port)
	errors.ExitIfErr(err)
	defer client.Close()

	err = client.SetBrightness(8)
	errors.ExitIfErr(err)

	dimension, err := client.GetDimension()
	errors.ExitIfErr(err)

	fmt.Printf("screen dimension is %dx%d\n", dimension.Width, dimension.Height)

	//scrobe(client)
	//scanLines(client, dimension)
	playGif(client, dimension)
	//drawTest(client, dimension)
	//drawHue(client)
	// drawGradient(client, dimension)

	//WaitForCtrlC()
}

func drawGradient(client pi_leds.Client, dimension *pi_proto.DimensionResponse) {
	rgb := color.Black.RGB()

	client.SetBrightness(255)

	for x := int16(0); x < int16(dimension.Width); x++ {
		client.DrawLine(x, 0, x, int16(dimension.Height), rgb)
		rgb.B += (256 / 64)
	}
	client.UpdateScreen()
}

func drawHue(client *pi_leds.ProtoClient) {
	hue := color.Red.HSL()

	for {
		hue.H = (hue.H + 1) % 255
		client.Clear(hue)
		client.UpdateScreen()
		time.Sleep(time.Second / 20)
	}

}

func drawTest(client *pi_leds.ProtoClient, dimension *pi_proto.DimensionResponse) {
	err := client.Clear(color.Yellow)
	errors.PrintIfErr(err)

	w, h := int(dimension.Width), int(dimension.Height)
	//fw, fh := float64(w), float64(h)

	ctx := gg.NewContext(w, h)

	ctx.SetColor(color.Green)
	ctx.SetPixel(0, 0)

	ctx.SetColor(color.Blue)
	ctx.SetPixel(w-1, 0)

	ctx.SetColor(color.Cyan)
	ctx.SetPixel(0, h-1)

	ctx.SetColor(color.Red)
	ctx.SetPixel(w-1, h-1)

	err = client.DrawImg(ctx.Image())
	errors.PrintIfErr(err)

	err = client.UpdateScreen()
	errors.PrintIfErr(err)

	WaitForCtrlC()
}

func playGif(client pi_leds.Client, dimension *pi_proto.DimensionResponse) {

	fh, err := os.Open("gif/Blinky2.gif")
	//fh, err := os.Open("gif/blocks1.gif")
	errors.ExitIfErr(err)

	anim, err := gif.DecodeAll(fh)
	errors.ExitIfErr(err)

	err = client.Clear(color.Black)
	errors.PrintIfErr(err)

	w := int(dimension.Width)
	h := int(dimension.Height)

	ctx := gg.NewContext(w, h)

	for {
		for i, gifImg := range anim.Image {

			//err = client.SetBrightness(8)
			//errors.ExitIfErr(err)

			//println("color model:", reflect.TypeOf(gifImg.ColorModel()).Name())

			ctx.DrawImage(gifImg, 0, 0)
			//ctx.SavePNG(fmt.Sprintf("test%d.png", i))

			delay := anim.Delay[i]
			proto.Int(delay)
			//ch := time.After(time.Duration(delay) * time.Second / 100)
			ch := time.After(time.Second / 120)

			err = client.DrawImg(ctx.Image())
			//err = client.DrawImgRaw(ctx.Image(), mosaic)
			errors.PrintIfErr(err)

			err = client.UpdateScreen()
			errors.PrintIfErr(err)

			<-ch
		}
	}
}

func scanLines(client pi_leds.Client, dimension *pi_proto.DimensionResponse) {
	colors := []color.Color{
		color.Red,
		color.Green,
		color.Blue,
	}

	horizontal := true
	it := int16(0)
	col := 0

	for {
		err := client.Clear(colors[col])
		errors.PrintIfErr(err)

		count := dimension.Width
		if !horizontal {
			count = dimension.Height
		}

		if horizontal {
			err = client.DrawLine(it, 0, it, int16(dimension.Height), color.White)
			errors.PrintIfErr(err)
		} else {
			err = client.DrawLine(0, it, int16(dimension.Width), it, color.White)
			errors.PrintIfErr(err)
		}

		it++

		if it >= int16(count) {
			horizontal = !horizontal
			it = 0
			col = (col + 1) % len(colors)
		}

		err = client.UpdateScreen()
		errors.PrintIfErr(err)

		//time.Sleep(time.Second / 10)
	}
}

func scrobe(client pi_leds.Client) {
	colors := []color.Color{color.Black, color.White, color.Black, color.Red, color.Black, color.Blue, color.Black, color.Green}

	idx := 0
	for {
		idx = (idx + 1) % len(colors)

		err := client.Clear(colors[idx])
		errors.PrintIfErr(err)

		err = client.UpdateScreen()
		errors.PrintIfErr(err)

		//time.Sleep(time.Millisecond * 17)
	}
}

func WaitForCtrlC() {
	var end_waiter sync.WaitGroup
	end_waiter.Add(1)
	var signal_channel chan os.Signal
	signal_channel = make(chan os.Signal, 1)
	signal.Notify(signal_channel, os.Interrupt)
	go func() {
		<-signal_channel
		end_waiter.Done()
	}()
	end_waiter.Wait()
}

type DnsEntry struct {
	Id   string
	IP   net.IP
	Port int
}
