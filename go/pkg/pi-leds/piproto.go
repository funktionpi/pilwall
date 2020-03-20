package pileds

import (
	"encoding/binary"
	"fmt"
	"image"
	"net"
	"sync"
	"time"

	"github.com/draeron/gopkg/chrono"
	"github.com/draeron/gopkg/color"
	"github.com/draeron/gopkg/errors"
	"github.com/funktionpi/pilwall/go/pkg/api/pi-proto"
	"github.com/funktionpi/pilwall/go/pkg/layout"
	"github.com/fogleman/gg"

	"github.com/golang/protobuf/proto"
)

//go:generate protoc -I ../../../slave/lib/proto -I $NANOPB/generator/proto --go_out=../api/pi-proto pi.proto

type Proto struct {
	udpsock net.Conn

	lastImg   image.Image
	responses sync.Map
	next      int32
}

func ConnectProto(ip net.IP, port int) (*Proto, error) {
		c := &Proto{
			next: 1,
		}

	fmt.Printf("connecting to pi-leds '%s'\n", ip.String())
	conn, err := net.DialTimeout("udp", fmt.Sprintf("%s:%d", ip.String(), port), time.Second*3)
	if err != nil {
		return nil, err
	}
	c.udpsock = conn

	go c.processResponse()
	return c, nil
}

func (c *Proto) Close() {
	err := c.udpsock.Close()
	errors.PrintIfErr(err)
}

func (c *Proto) processResponse() {
	for {
		var err error
		buf := []byte{}

		tmp := [128]byte{}
		size := 0
		size, err = c.udpsock.Read(tmp[0:])
		buf = tmp[0:size]

		if err != nil {
			errors.PrintIfErr(err)
			continue
		}

		resp := pi_proto.Response{}
		err = proto.Unmarshal(buf, &resp)
		if err != nil {
			errors.PrintIfErr(err)
		} else {
			if it, ok := c.responses.Load(resp.Id); ok {
				ch := it.(*chan *pi_proto.Response)
				*ch <- &resp
				c.responses.Delete(resp.Id)
			} else {
				fmt.Printf("could not find channel for request %d\n", resp.Id)
			}
		}
	}
}

func (c *Proto) SetBrightness(brightness uint8) error {
	//fmt.Printf("setting brightness to %d\n", brightness)

	req := pi_proto.Request_Brightness{&pi_proto.BrightnessRequest{
		Brightness: int32(brightness),
	}}

	_, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: &req}))
	return err
}

func (c *Proto) Clear(color color.Color) error {
	val := convertColor(color)
	//fmt.Printf("Clear color: %v\n", color.RGB())

	req := pi_proto.Request_Clear{&pi_proto.ClearRequest{
		Color: val,
	}}
	_, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: &req}))
	return err
}

func (c *Proto) GetDimension() (*pi_proto.DimensionResponse, error) {
	req := pi_proto.Request_Dimension{&pi_proto.DimensionRequest{}}
	msg, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: &req}))
	if err != nil {
		return nil, err
	}

	pkg, ok := msg.Response.(*pi_proto.Response_Dimension)
	if !ok {
		return nil, fmt.Errorf("wrong resonse datatype")
	}

	return pkg.Dimension, nil
}

func (c *Proto) UpdateScreen() error {
	req := &pi_proto.Request_Update{
		Update: &pi_proto.UpdateRequest{},
	}

	_, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: req}))
	return err
}

func (c *Proto) DrawLine(x1, y1, x2, y2 int16, rgba color.Color) error {
	col := convertColor(rgba)
	fmt.Printf("Line from (%d,%d) to (%d,%d) => Color: %v\n", x1, y1, x2, y2, rgba)

	req := pi_proto.Request_DrawLine{&pi_proto.DrawLineRequest{
		Color: col,
		Start: convertCoord(x1, y1),
		End:   convertCoord(x2, y2),
	}}

	_, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: &req}))
	return err
}

func (c *Proto) DrawImgRaw(img image.Image, mosaic layout.Mosaic) error {
	raw := mosaic.SliceToBytes(img)
	index := 0

	sw := chrono.NewStopWatch(false)
	reqcount := 0

	maxlen := 128

	for len(raw) > 0 {
		req := pi_proto.Request_Raw{&pi_proto.RawRequest{
			Index: int32(index),
		}}

		count := 0
		if len(raw) > maxlen*3 {
			count = maxlen * 3 / 4
		} else {
			count = len(raw) / 4
		}
		index += count * 4 / 3

		for i := 0; i < count; i++ {
			req.Raw.Pixels = append(req.Raw.Pixels, int32(binary.LittleEndian.Uint32(raw[:4])))
			raw = raw[4:]
		}

		sw.Start()
		reqcount++
		_, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: &req}))
		//_, err := c.send(&req)
		sw.Stop()

		if err != nil {
			return err
		}
	}

	//qwe := sw.Elapsed().Milliseconds()
	//println("elapsed: ", qwe, "ms on", reqcount, "requests")

	return nil
}

func (c *Proto) DrawImg(img image.Image) error {
	req := pi_proto.Request_Pixels{&pi_proto.PixelsRequest{
		Pixels: []*pi_proto.PixelColor{},
	}}

	if c.lastImg == nil {
		c.lastImg = image.NewRGBA(img.Bounds())
	}

	maxcount := 96
	pixcount := 0
	reqcount := 0
	sw := chrono.NewStopWatch(false)

	w := img.Bounds().Max.X - img.Bounds().Min.X
	h := img.Bounds().Max.Y - img.Bounds().Min.Y

	for x := 0; x < w; x++ {
		for y := 0; y < h; y++ {
			newColor := color.FromColor(img.At(x, y))
			oldColor := color.FromColor(c.lastImg.At(x, y))
			if !oldColor.Equal(newColor) {
				req.Pixels.Pixels = append(req.Pixels.Pixels, &pi_proto.PixelColor{
					Color: convertColor(newColor),
					Coord: convertCoord(int16(x), int16(y)),
				})
			}

			if len(req.Pixels.Pixels) > 0 && (len(req.Pixels.Pixels) == maxcount || (x == w-1 && y == h-1)) {
				//fmt.Printf("sending packet, size %d, %d pixels\n", size, len(req.Pixels.Pixels))
				sw.Start()
				_, err := c.WaitForResponse(c.send(&pi_proto.Request{Request: &req}))
				//_, _, err := c.send(req)
				reqcount++
				pixcount += len(req.Pixels.Pixels)
				sw.Stop()

				if err != nil {
					return err
				}
				req.Pixels.Pixels = []*pi_proto.PixelColor{}
			}
		}
	}

	ctx := gg.NewContextForImage(c.lastImg)
	ctx.DrawImage(img, 0, 0)
	c.lastImg = ctx.Image()

	//if reqcount > 0 {
	//	qwe := sw.Elapsed().Milliseconds()
	//	println("elapsed: ", qwe, "ms on", reqcount, "requests with", pixcount, "pixels uploaded")
	//} else {
	//	println("no diff ?")
	//}

	return nil
}

func (c *Proto) WaitForResponse(respChan <-chan *pi_proto.Response, id int32, err error) (*pi_proto.Response, error) {
	if err != nil {
		return nil, err
	}

	// block until we received a response
	select {
	case response := <-respChan:
		//println("response received successfully")
		return response, nil
	case <-time.After(1 * time.Second):
		c.responses.Delete(id)
		return nil, fmt.Errorf("response timeout")
	}
}

func (c *Proto) send(req *pi_proto.Request) (<-chan *pi_proto.Response, int32, error) {
	req.Id = c.next
	defer func() { c.next++ }()

	respChan := make(chan *pi_proto.Response)
	c.responses.Store(c.next, &respChan)

	buf, err := proto.Marshal(req)
	if err != nil {
		return nil, c.next, err
	}

	_, err = c.udpsock.Write(buf)
	if err != nil {
		return nil, c.next, err
	}

	return respChan, c.next, err
}

// Downgrade 24-bit color to 16-bit (add reverse gamma lookup here?)
func convertColor(color color.Color) uint32 {
	r, g, b, _ := color.RGBA()
	return (uint32(r>>8) << 16) | (uint32(g>>8) << 8) | (uint32(b >> 8))
}

func convertCoord(x, y int16) *pi_proto.Coord {
	return &pi_proto.Coord{
		Xy: (int32(x) << 16) | int32(y),
	}
}
