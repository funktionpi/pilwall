package pileds

import (
	"fmt"
	"github.com/draeron/gopkg/color"
	"github.com/draeron/pi-leds/go/pkg/layout"
	"github.com/jsimonetti/go-artnet/packet"
	"image"
	"net"
	"time"
)

type Artnet struct {
	udpsock net.Conn
	mosaic layout.Mosaic
	pixelPerPacket int
}

const (
	ArtnetDefaultPort       = 6454
	ArtnetMaxBytesPerPacket = 512
)

func ConnectArtnet(ip net.IP, mosaic layout.Mosaic) (Client, error) {

	a := &Artnet{
		mosaic: mosaic,
		pixelPerPacket: int(mosaic.Count()),
	}

	var err error
	a.udpsock, err = net.DialTimeout("udp", fmt.Sprintf("%s:%d", ip.String(), ArtnetDefaultPort), time.Second*3)
	if err != nil {
		return  nil, err
	}

	// divide by 2 until it fits max bytes
	for a.pixelPerPacket * 3 > ArtnetMaxBytesPerPacket {
		a.pixelPerPacket /= 2
	}

	return a, nil
}

func (a *Artnet) Close() {
	if a.udpsock != nil {
		a.udpsock.Close()
		a.udpsock = nil
	}
}

// TODO: reduce brigthness client side ?
func (a *Artnet) SetBrightness(brightness uint8) error {
	return nil
}

// TODO: send a clear image?
func (a *Artnet) Clear(color color.Color) error {
	return nil
}

func (a *Artnet) UpdateScreen() error {
	// nothing to do
	return nil
}

func (a *Artnet) DrawImgRaw(img image.Image, mosaic layout.Mosaic) error {
	data := mosaic.SliceToBytes(img)

	universe := uint8(1)
	seq := uint8(1)

	for len(data) > 0 {
		packet := packet.ArtDMXPacket{
			Sequence: seq,
			SubUni: universe,
			Length: uint16(a.pixelPerPacket * 3),
		}
		copy(packet.Data[:], data[:a.pixelPerPacket*3])

		out, err := packet.MarshalBinary()
		if err != nil {
			return err
		}

		_, err = a.udpsock.Write(out)
		if err != nil {
			return err
		}

		data = data[a.pixelPerPacket*3:]
		universe++
		seq++
	}

	return nil
}

func (a *Artnet) DrawImg(img image.Image) error {
	return a.DrawImgRaw(img, a.mosaic)
}

