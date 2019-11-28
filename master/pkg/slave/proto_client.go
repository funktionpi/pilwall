package slave

import (
	"encoding/binary"
	"fmt"
	"github.com/fogleman/gg"
	"image"
	"led-wall/pkg/chrono"
	"led-wall/pkg/color"
	"led-wall/pkg/errors"
	"led-wall/pkg/layout"
	"net"
	"net/url"
	"sync"
	"time"

	"github.com/golang/protobuf/proto"
)

//go:generate protoc -I ../../../slave/lib/proto -I $NANOPB/generator/proto --go_out=. ledctrl.proto

type ProtoClient struct {
	udpsock net.Conn

	lastImg image.Image
	responses sync.Map
	next int32
}

func ConnectProto(url url.URL) (*ProtoClient, error) {
	c := &ProtoClient{
		next: 1,
	}

	fmt.Printf("connecting to ledslave '%s'\n", url.String())
	conn, err := net.DialTimeout("udp", url.Host + ":2001", time.Second*3)
	if err != nil {
		return nil, err
	}
	c.udpsock = conn

	go c.processResponse()
	return c, nil
}

func (c *ProtoClient) Close() {
	err := c.udpsock.Close()
	errors.PrintIfErr(err)
}

func (c *ProtoClient) processResponse() {
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

		resp := Response{}
		err = proto.Unmarshal(buf, &resp)
		if err != nil {
			errors.PrintIfErr(err)
		} else {
			if it, ok := c.responses.Load(resp.Id); ok {
				ch := it.(*chan *Response)
				*ch <- &resp
				c.responses.Delete(resp.Id)
			} else {
				fmt.Printf("could not find channel for request %d\n", resp.Id)
			}
		}
	}
}

func (c *ProtoClient) SetBrightness(brightness uint8) error {

	fmt.Printf("setting brightness to %d\n", brightness)

	req := &Request_Brightness{&BrightnessRequest{
		Brightness: int32(brightness),
	}}

	_, err := WaitForResponse(c.send(req))
	return err
}

func (c *ProtoClient) Clear(color color.Color) error {
	val := convertColor(color)
	fmt.Printf("Clear color: %v\n", color.RGB())

	req := &Request_Clear{&ClearRequest{
		Color: val,
	}}
	_, err := WaitForResponse(c.send(req))
	return err
}

func (c *ProtoClient) GetDimension() (*DimensionResponse, error) {
	req := &Request_Dimension{&DimensionRequest{}}
	msg, err := WaitForResponse(c.send(req))
	if err != nil {
		return nil, err
	}

	pkg, ok := msg.Response.(*Response_Dimension)
	if !ok {
		return nil, fmt.Errorf("wrong resonse datatype")
	}

	return pkg.Dimension, nil
}

func (c *ProtoClient) UpdateScreen() error {
	req := &Request_Update{
		Update: &UpdateRequest{},
	}

	_, err := WaitForResponse(c.send(req))
	return err
}

func (c *ProtoClient) DrawLine(x1, y1, x2, y2 int16, rgba color.Color) error {
	col := convertColor(rgba)
	fmt.Printf("Line from (%d,%d) to (%d,%d) => Color: %v\n", x1, y1, x2, y2, rgba)

	req := &Request_DrawLine{&DrawLineRequest{
		Color: col,
		Start: convertCoord(x1, y1),
		End:   convertCoord(x2, y2),
	}}

	_, err := WaitForResponse(c.send(req))
	return err
}

func (c *ProtoClient) DrawImgRaw(img image.Image, mosaic layout.Mosaic) error {
	raw := mosaic.SliceToBytes(img)
	index := 0

	sw := chrono.NewStopWatch(false)
	reqcount := 0

	maxlen := 128

	for len(raw) > 0 {
		req := Request_Raw{&RawRequest{
			Index: int32(index),
		}}

		count := 0
		if len(raw) > maxlen * 3 {
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
		_, err := WaitForResponse(c.send(&req))
		//_, err := c.send(&req)
		sw.Stop()

		if err != nil {
			return err
		}
	}

	qwe := sw.Elapsed().Milliseconds()
	println("elapsed: ", qwe, "ms on", reqcount, "requests")

	return nil
}

func (c *ProtoClient) DrawImg(img image.Image) error {
	req := &Request_Pixels{&PixelsRequest{
		Pixels: []*PixelColor{},
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
				req.Pixels.Pixels = append(req.Pixels.Pixels, &PixelColor{
					Color: convertColor(newColor),
					Coord: convertCoord(int16(x), int16(y)),
				})
			}

			if len(req.Pixels.Pixels) > 0 && (len(req.Pixels.Pixels) == maxcount || (x == w-1 && y == h-1)) {
				//fmt.Printf("sending packet, size %d, %d pixels\n", size, len(req.Pixels.Pixels))
				sw.Start()
				_, err := WaitForResponse(c.send(req))
				reqcount++
				pixcount += len(req.Pixels.Pixels)
				sw.Stop()

				if err != nil {
					return err
				}
				req.Pixels.Pixels = []*PixelColor{}
			}
		}
	}

	//c.lastImg = img
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

func WaitForResponse(respChan <-chan *Response, err error) (*Response, error) {
	if err != nil {
		return nil, err
	}

	// block until we received a response
	select {
	case response := <-respChan:
		//println("response received successfully")
		return response, nil
	case <-time.After(1 * time.Second):
		return nil, fmt.Errorf("response timeout")
	}
}

func (c *ProtoClient) send(req isRequest_Request) (<-chan *Response, error) {

	msg := Request{
		Id:      c.next,
		Request: req,
	}

	respChan := make(chan *Response)
	c.responses.Store(c.next, &respChan)
	c.next++

	buf, err := proto.Marshal(&msg)
	if err != nil {
		return nil, err
	}

	_, err = c.udpsock.Write(buf)
	if err != nil {
		return nil, err
	}

	return respChan, err
}

// Downgrade 24-bit color to 16-bit (add reverse gamma lookup here?)
func convertColor(color color.Color) uint32 {
	r, g, b, _ := color.RGBA()
	return (uint32(r>>8) << 16) | (uint32(g>>8) << 8) | (uint32(b >> 8))
}

func convertCoord(x, y int16) *Coord {
	return &Coord{
		Xy: (int32(x) << 16) | int32(y),
	}
}
