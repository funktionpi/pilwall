package main

import (
	"fmt"
	"image/gif"
	"net"
	"os"
	"os/signal"
	"sync"
	"time"

	"github.com/funktionpi/pilwall/go/pkg/api/pi-proto"
	"github.com/funktionpi/pilwall/go/pkg/discovery"
	"github.com/funktionpi/pilwall/go/pkg/layout"
	"github.com/funktionpi/pilwall/go/pkg/pi-leds"
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
			Dimension: svr.TileDimension,
			Layout:    layout.MustParseLayout(svr.PanelLayout),
		},
	}

	client, err := pileds.ConnectProto(svr.Ip, svr.Port)
	//client, err := pileds.ConnectTpm2(svr.Ip, mosaic)
	//client, err := pileds.ConnectArtnet(svr.Ip, mosaic)
	errors.ExitIfErr(err)
	defer client.Close()

	err = client.SetBrightness(8)
	errors.ExitIfErr(err)

	fmt.Printf("screen dimension is %dx%d\n", mosaic.Width(), mosaic.Height())

	//playGif(client, mosaic)
	scrobe(client, mosaic)
	//WaitForCtrlC()
}

func drawHue(client *pileds.Proto) {
	hue := color.Red.HSL()

	for {
		hue.H = (hue.H + 1) % 255
		client.Clear(hue)
		client.UpdateScreen()
		time.Sleep(time.Second / 20)
	}

}

func drawTest(client *pileds.Proto, dimension *pi_proto.DimensionResponse) {
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

func playGif(client pileds.Client, mosaic layout.Mosaic) {

	fh, err := os.Open("Blinky2.gif")
	//fh, err := os.Open("gif/blocks1.gif")
	errors.ExitIfErr(err)

	anim, err := gif.DecodeAll(fh)
	errors.ExitIfErr(err)

	err = client.Clear(color.Black)
	errors.PrintIfErr(err)

	w := int(mosaic.Width())
	h := int(mosaic.Height())

	ctx := gg.NewContext(w, h)

	for {
		for i, gifImg := range anim.Image {

			err = client.SetBrightness(8)
			errors.ExitIfErr(err)

			//println("color model:", reflect.TypeOf(gifImg.ColorModel()).Name())

			ctx.DrawImage(gifImg, 0, 0)
			//ctx.SavePNG(fmt.Sprintf("test%d.png", i))

			delay := anim.Delay[i]
			proto.Int(delay)
			//ch := time.After(time.Duration(delay) * time.Second / 100)
			ch := time.After(time.Second / 60)

			err = client.DrawImg(ctx.Image())
			//err = client.DrawImgRaw(ctx.Image(), mosaic)
			errors.PrintIfErr(err)

			err = client.UpdateScreen()
			errors.PrintIfErr(err)

			<-ch
		}
	}
}

func scrobe(client pileds.Client, mosaic layout.Mosaic) {
	colors := []color.Color{color.Black, color.White, color.Black, color.Red, color.Black, color.Blue, color.Black, color.Green}

	w := int(mosaic.Width())
	h := int(mosaic.Height())
	ctx := gg.NewContext(w, h)

	var err error

	idx := 0
	for {
		idx = (idx + 1) % len(colors)

		ctx.SetColor(colors[idx])
		ctx.Clear()

		//err = client.Clear(colors[idx])
		//errors.PrintIfErr(err)

		err = client.DrawImg(ctx.Image())
		errors.PrintIfErr(err)

		err = client.UpdateScreen()
		errors.PrintIfErr(err)

		time.Sleep(time.Second / 60)
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
